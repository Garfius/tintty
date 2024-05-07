#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch from TFT_eSPI
#include "input.h"
#include "tintty.h"
/**
 * TinTTY main sketch
 * by Nick Matantsev 2017 & Gerard Forcada 2024
 *
 * Original reference: VT100 emulation code written by Martin K. Schroeder
 * and modified by Peter Scargill.
 * 
 * to-do:
 *  Test on the original ILI9341
 *  Port back to AVR if possible
 *  Scroll back
 *  Improve multi-core, use myCheesyFB.outputting so refresh while receiving
 *  
 */
#define snappyMillisLimit 185// idle refresh time
#define LOCAL_BUFFER_SIZE 800
#define tintty_baud_rate 9600

volatile static char myCharBuffer[LOCAL_BUFFER_SIZE];// whole ram must be buffer, lol
volatile bool running = false;

struct tintty_display ili9341_display = {// from serial to display from ~236 tintty_idle(&ili9341_display)
  ILI9341_WIDTH,// x
  (ILI9341_HEIGHT - KEYBOARD_HEIGHT), // y 
  ILI9341_WIDTH / TINTTY_CHAR_WIDTH, // colCount
  (ILI9341_HEIGHT - KEYBOARD_HEIGHT) / TINTTY_CHAR_HEIGHT, // rowCount

  [](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){//fill_rect, cursor
    assureRefreshArea(x, y, w, h);
    spr.fillRect(x, y, w, h, color);
  },

  [](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels){//draw_pixels, no es fa servir
    assureRefreshArea(x, y, w, h);
    spr.setAddrWindow(x, y, x + w - 1, y + h - 1);
    spr.pushColors(pixels, w * h, 1);
  },

  [](int16_t offset){//set_vscroll
    
  }
};
// buffer to test various input sequences
class CharBuffer {
private:
public:
    
    volatile int head = 0;
    volatile int tail = 0;

    // Add a character to the buffer
    void addChar(char c) {
        if (((tail + 1) % LOCAL_BUFFER_SIZE) == head) { // Buffer is full
          giveErrorVisibility(false);
        }
        myCharBuffer[tail] = c;
        tail = (tail + 1) % LOCAL_BUFFER_SIZE;
    }

    // Consume a character from the buffer
    char consumeChar() {
        if (head == tail) { // Buffer is empty
          giveErrorVisibility(false);
        }
        char c = myCharBuffer[head];
        head = (head + 1) % LOCAL_BUFFER_SIZE;
        return c;
    }
};
// passa el tros indicat a;
void refreshDisplayIfNeeded(){
  // si hasChanges i myCheesyFB.lastRemoteDataTime < millis();
  if(!myCheesyFB.hasChanges)return;
  if(millis() > (myCheesyFB.lastRemoteDataTime+snappyMillisLimit)){
    myCheesyFB.outputting = true;
    spr.pushSprite(myCheesyFB.minX,myCheesyFB.minY,myCheesyFB.minX,myCheesyFB.minY,myCheesyFB.maxX-myCheesyFB.minX,myCheesyFB.maxY-myCheesyFB.minY);
    myCheesyFB.outputting = false;
    myCheesyFB.hasChanges = false;
    myCheesyFB = fameBufferControl{UINT16_MAX,0,UINT16_MAX,0, false,false,0};
  }
}
CharBuffer buffer;
void loop1()
{
    if((userTty->available() > 0)){
        buffer.addChar((char)userTty->read());
    }
    if(buffer.head != buffer.tail)myCheesyFB.lastRemoteDataTime = millis();
}
void loop()
{
	
		 tintty_run(// serial to Screen
    [](){
      // peek idle loop, non blocking?
      while (true) {
        //bufferAtoB();
        
          if(buffer.head != buffer.tail)return myCharBuffer[buffer.head];
          tintty_idle(&ili9341_display);
          if (myCheesyFB.hasChanges) {
            refreshDisplayIfNeeded();
          }else{
            input_idle();
          }
        
      }
    },
    [](){
      while(true) {// read char
        //bufferAtoB();
        tintty_idle(&ili9341_display);
        
          if(buffer.head != buffer.tail)return buffer.consumeChar();
          if (myCheesyFB.hasChanges) {
            refreshDisplayIfNeeded();
          }else{
            input_idle();
          }
        
      }
    },//send char
    [](char ch){
      userTty->write(ch);
    }
    ,
    &ili9341_display
  );
}



/**
 * NOT optimized AT ALL! just debugged
*/
void tft_espi_calibrate_touch(){
  uint16_t calibrationData[7];// els 2 extra son per fer el check de si es vol fer a mà
  uint8_t *calibrationDataBytePoint = (uint8_t *)calibrationData;
  uint8_t calDataOK = 0;
  uint8_t calDataTst = 0;
  //-------------------eeprom integrity check 
  calDataOK = EEPROM.read(0);
  calDataTst = ~EEPROM.read(1);
  calDataOK = calDataOK == calDataTst;
  int eePos = 2;
  if (calDataOK && (!tft.getTouch(&calibrationData[5], &calibrationData[6]))) {
    // calibration data valid & NO TSOLICITED
    for(int i = 0; i < 10; i++) {
      calDataTst = EEPROM.read(eePos+i);
      calibrationDataBytePoint[i] = calDataTst;
      eePos++;
    }
    tft.setTouch(calibrationData);
  } else {
    if(tft.getTouch(&calibrationData[5], &calibrationData[6])){
      tft.fillScreen(TFT_YELLOW);  
      delay(1000);
      tft.fillScreen(TFT_BLACK);  
    }
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    while(calDataOK == EEPROM.read(0))calDataOK = (uint8_t)random(0,255); // firma
    EEPROM.write(0,calDataOK);
    calDataTst = ~calDataOK;
    EEPROM.write(1,calDataTst);
    int eePos = 2;
    for(int i = 0; i < 10; i++) {
      calDataTst = calibrationDataBytePoint[i];
      EEPROM.write(eePos+i, calDataTst);
      eePos++;
    }
    EEPROM.commit();
    EEPROM.end();
  }
  tft.fillScreen(TFT_BLACK);
}
void setup1() {
  while(!running){}
}

void setup() {
  //-----------------------setup
  pinMode(23,OUTPUT);// millora 3.3v GPIO23 controls the RT6150 PS (Power Save) pin. When PS is low (the default on Pico) 
  digitalWrite(23, HIGH);
  
  Serial1.begin(tintty_baud_rate,SERIAL_8N1);
  EEPROM.begin(255);
  tft.begin();
  
  gpio_pull_up(2);// ensure pull-up for receiving wire
  
  userTty = &Serial1; // assign receiving serial port
  
  //-----------------------init
  giveErrorVisibility(true);
  
  tft.setFreeFont(GLCD);
  tft.setTextSize(1);
  
  spr.setFreeFont(GLCD);
  spr.setColorDepth(8);
  
  if(spr.createSprite(ILI9341_WIDTH, (ILI9341_HEIGHT - KEYBOARD_HEIGHT)) == nullptr)giveErrorVisibility(false);
  
  spr.setTextSize(1);
  spr.fillSprite(TFT_BLACK);

  tft_espi_calibrate_touch();
  
  input_init();

  //---------------go!
  running = true;
}


#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
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
 * -use sprite as frameBuffer, and update only  changed area
 * -go back to dma somehow
 * -cursor blink
 * 
 */
/*
struct fameBufferControl {
    uint16_t minX,maxX,minY,maxY;
    bool outputting;
    bool hasChanges;
    bool refreshNeeded;
    unsigned int refreshTime;
};
*/
// buffer size and beheaviour
// #define SERIAL_RX_BUFFER_SIZE 255

#define snappyMillisLimit 75// idle refresh time
#define LOCAL_BUFFER_SIZE 800
#define bufferProcessingBlockSize 32

/*
#define bufferLimitNoProcess 250
#define bufferLimitUserEcho 50
#define bufferLimitTtyBusy 500
#define multicore false
#define multicoreRefreshTime 700*/
static char myCharBuffer[LOCAL_BUFFER_SIZE];// whole ram must be buffer, lol
//unsigned int lastRefresh,lastBufferEmpty,lastUserInput;
char charTmp;

//enum ttyStateList {snappy,waiting4FufferEmpty,busy};
//ttyStateList ttyState = snappy;
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
    //tft.vertScroll(0, (ILI9341_HEIGHT - KEYBOARD_HEIGHT), offset);
    spr.fillRect(0, 0, ILI9341_WIDTH,  (ILI9341_HEIGHT - KEYBOARD_HEIGHT), TFT_BLACK);
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
        if ((tail + 1) % LOCAL_BUFFER_SIZE == head) { // Buffer is full
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
    u_int16_t size(){
      if(head < tail ){
        return tail-head;
      }else if(head > tail){
        return (LOCAL_BUFFER_SIZE-head)+tail;
      }else return 0;
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
    myCheesyFB = fameBufferControl{UINT16_MAX,0,UINT16_MAX,0, false,false,false,0};
  }
}
CharBuffer buffer;
/**
 * Aqui llegeix de entrada fins[midaBlock]
*/
void bufferAtoB(){ // 227
  if (serialEventRun) serialEventRun(); 
  while(userTty->available() > 0) {
    myCheesyFB.lastRemoteDataTime = millis();
    charTmp = (char)userTty->read();
    buffer.addChar(charTmp);
    //if(millis() > (myCheesyFB.lastRemoteDataTime+snappyMillisLimit))break;// ja has esperat snappy desde ultim byte tens 1 core
  }
  myCheesyFB.processingBlock = buffer.size() > bufferProcessingBlockSize;
  // snappyMillisLimit !!!
  // refrescar-> rebut_algo&&snappy, waiting4FufferEmpty&&(buffer.size()==0), busy&&(buffer.size()==0), multicore&&(multicoreRefreshTime lastRefresh)
  // que qualsevol daquests dispari un refresh en snappyMillisLimit
  //if (serialEventRun) serialEventRun(); 
}

/**
 * NOT optimized AT ALL! just debugged
*/
void tft_espi_calibrate_touch(){
  uint16_t calibrationData[7];// els 2 extra son per fer el check de si es vol fer a mà
  uint8_t *calibrationDataBytePoint = (uint8_t *)calibrationData;
  uint8_t calDataOK = 0;
  uint8_t calDataTst = 0;
  //-------------------eeprom inici
  
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
void setup() {
  //-----------------------Serial port setup
  Serial1.begin(9600);
  userTty = &Serial1;
  //-----------------------
  giveErrorVisibility(true);
  //myCheesyFB.doing = tinttyProcessingState::idle;
  EEPROM.begin(255);
  tft.begin();
  // AQUI TENS inicialitzacions -- SPRITE NEW 
  // check nullptr
  // ubicar declaracions de mides
  tft.setFreeFont(GLCD);
  tft.setTextSize(1);
  spr.setFreeFont(GLCD);
  spr.setTextSize(1);
  if(spr.createSprite(ILI9341_WIDTH, (ILI9341_HEIGHT - KEYBOARD_HEIGHT)) == nullptr)giveErrorVisibility(false);
  spr.fillSprite(TFT_BLACK);
  
  tft_espi_calibrate_touch();
  input_init();

  tintty_run(// de serial cap a la pantalla, pots desactivar teclat -- @todo
    [](){
      // peek idle loop, non blocking?
      while (true) {
        bufferAtoB();
        if(myCheesyFB.processingBlock){
          if(buffer.head != buffer.tail){// hi ha quelcom a fer?
              //userTty->write(buffer.myCharBuffer[buffer.head]);
              return myCharBuffer[buffer.head];
          }else{
            myCheesyFB.processingBlock= false;
          }
        }else{
          if(buffer.head != buffer.tail)return myCharBuffer[buffer.head];
          tintty_idle(&ili9341_display);
          if (userTty->available() == 0) {
            refreshDisplayIfNeeded();
            input_idle();
          }
        }
      }
    },
    [](){
      while(true) {// read char
        bufferAtoB();
        tintty_idle(&ili9341_display);// no ha de fer cursor
        if(myCheesyFB.processingBlock){
          if(buffer.head != buffer.tail){// hi ha quelcom a fer?
              //userTty->write(buffer.myCharBuffer[buffer.head]);
              return buffer.consumeChar();
          }else{
            myCheesyFB.processingBlock= false;
          }
        }else{
          if(buffer.head != buffer.tail)return buffer.consumeChar();
          if (userTty->available() == 0) {
            refreshDisplayIfNeeded();
            input_idle();
          }
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

void loop() {
}
//the variables memory is recovered automatically at the end of the loop.


//#define SERIAL_RX_BUFFER_SIZE 255
#define LOCAL_BUFFER_SIZE 800
#include <Arduino.h>
#include <LittleFS.h>
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
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "input.h"
#include "tintty.h"
#define TFT_BLACK 0x0000
#define latenciaPantalla 250
struct tintty_display ili9341_display = {
  ILI9341_WIDTH,
  (ILI9341_HEIGHT - KEYBOARD_HEIGHT),
  ILI9341_WIDTH / TINTTY_CHAR_WIDTH,
  (ILI9341_HEIGHT - KEYBOARD_HEIGHT) / TINTTY_CHAR_HEIGHT,

  [](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    tft.fillRect(x, y, w, h, color);
  },

  [](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels){
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors(pixels, w * h, 1);
  },

  [](int16_t offset){
    //tft.vertScroll(0, (ILI9341_HEIGHT - KEYBOARD_HEIGHT), offset);
    tft.fillRect(0, 0, ILI9341_WIDTH,  (ILI9341_HEIGHT - KEYBOARD_HEIGHT), TFT_BLACK);
  }
};
// buffer to test various input sequences
static char myCharBuffer[LOCAL_BUFFER_SIZE];// whole ram must be buffer, lol
char charTmp;
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
};
CharBuffer buffer;
unsigned int nextRefresh,bufferIndex;
//CircularBuffer<char,LOCAL_BUFFER_SIZE> buffer;
#define nextByteWait 5
bool canviat;
void checkBuffer(){
  canviat = false;
  if(userTty->available() > 0) {
      nextRefresh = millis()+nextByteWait;
      while(true){
        if(userTty->available() > 0){
          charTmp = (char)userTty->read();
          buffer.addChar(charTmp);
          nextRefresh = millis()+nextByteWait;
        }
        if(nextRefresh < millis())break;
      }
      //buffer.push((char)userTty->read());
      canviat = true;
  }
  if(canviat){
    nextRefresh = millis() + latenciaPantalla;
    if (serialEventRun) serialEventRun();
  }
}
#define CALIBRATION_FILE "/calibrationData"
/**
 * eeprom.h for avr
 * el segon ha de ser el negat del primer
*/
void tft_espi_calibrate_touch(){
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  if (!LittleFS.begin()){
    giveErrorVisibility(false);
  }

  bool tmp = tft.getTouch(&calibrationData[0], &calibrationData[1]);
  // check if calibration file exists
  if (LittleFS.exists(CALIBRATION_FILE)) {
    File f = LittleFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }
  
  if (calDataOK || tmp) {
    // calibration data valid
    tft.setTouch(calibrationData);
  } else {
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
    tft.fillScreen(TFT_BLACK);
  }

}
void setup() {
  //-----------------------Serial port setup
  Serial1.begin(9600);
  userTty = &Serial1;
  //-----------------------
  giveErrorVisibility(true);
  tft.begin();
  tft.setFreeFont(GLCD);
  tft.setTextSize(1);
  tft_espi_calibrate_touch();
  input_init();

  tintty_run(
    [](){
      // peek idle loop
      while (true) {
        checkBuffer();
        if(nextRefresh < millis()){
          input_idle();
          //if(!buffer.isEmpty()){            return buffer.first();
          if(buffer.head != buffer.tail){// hi ha quelcom a fer?
              //userTty->write(buffer.myCharBuffer[buffer.head]);
              return myCharBuffer[buffer.head];
          }else{
            tintty_idle(&ili9341_display);
          }
        }
        
      }
    },
    [](){
      while(true) {// read char
        checkBuffer();
        if(nextRefresh < millis()){ 
          input_idle();
          //if(!buffer.isEmpty()){            return buffer.shift();
          if(buffer.head != buffer.tail){// hi ha quelcom a fer?
              //userTty->write('-');
              return buffer.consumeChar();
          }else{
            tintty_idle(&ili9341_display);
          }
        }
        
      }
    },//send char
    [](char ch){
      checkBuffer();
      userTty->write(ch);
    }
    ,
    &ili9341_display
  );
}

void loop() {
}
//the variables memory is recovered automatically at the end of the loop.


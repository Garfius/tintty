//#define SERIAL_RX_BUFFER_SIZE 255
#define LOCAL_BUFFER_SIZE 800
#include <Arduino.h>
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
//#include <CircularBuffer.hpp>
#include "input.h"
#include "tintty.h"
#define TFT_BLACK 0x0000
#define latenciaPantalla 250
/*
  void (*fill_rect)(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void (*draw_pixels)(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels);
  void (*set_vscroll)(int16_t offset); // scroll offset for entire screen
*/
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
/*
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}*/
/*
unsigned long FirstAdressHeap;
unsigned long getLatestHeapAdress() {
  int* heapVar=(int*)calloc(1,sizeof(int));
  unsigned long heapAdress = (unsigned long)&heapVar;
  free(heapVar);
  return heapAdress;
}*/

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
        Serial.write('+');
        if ((tail + 1) % LOCAL_BUFFER_SIZE == head) { // Buffer is full
        }
        Serial.println(tail);
        myCharBuffer[tail] = c;
        tail = (tail + 1) % LOCAL_BUFFER_SIZE;
    }

    // Consume a character from the buffer
    char consumeChar() {
        Serial.write('-');
        if (head == tail) { // Buffer is empty
        }
        Serial.println(head);
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
 * spiffs for rp2040
 * eeprim.h for avr
*/
void tft_espi_calibrate_touch(){
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  if (!SPIFFS.begin()) {
    //Serial.println("formatting file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }
  if (calDataOK) {
    // calibration data valid
    tft.setTouch(calibrationData);
  } else {
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }

}
void setup() {
  //-----------------------Serial port setup
  Serial1.begin(9600);
  userTty = &Serial1;
  //-----------------------
  
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
              //Serial.write(buffer.myCharBuffer[buffer.head]);
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
              //Serial.write('-');
              return buffer.consumeChar();
          }else{
            tintty_idle(&ili9341_display);
          }
        }
        
      }
    },//send char
    [](char ch){
      checkBuffer();
      userTty->print(ch);
    }
    ,
    &ili9341_display
  );
}

void loop() {
}
//the variables memory is recovered automatically at the end of the loop.


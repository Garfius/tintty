#define SERIAL_RX_BUFFER_SIZE 255
#define LOCAL_BUFFER_SIZE 1024
#include <Arduino.h>
/**
 * TinTTY main sketch
 * by Nick Matantsev 2017 & Gerard Forcada 2024
 *
 * Original reference: VT100 emulation code written by Martin K. Schroeder
 * and modified by Peter Scargill.
 * 
 * to-do:
 * eeprom checkDoCalibration
 * Touch pints
 *   240->
 *     2+20+2
 *   320->
 *     2+28+2
 * Choose serial port used via:
 *   Stream _serial
 def calculate_distance(point1, point2):
    x1, y1 = point1
    x2, y2 = point2
    return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
 */

#include <SPI.h>

#include <CircularBuffer.hpp>
#include "input.h"
#include "tintty.h"
#define latenciaPantalla 250

// using stock MCUFRIEND 2.4inch shield
MCUFRIEND_kbv tft;

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
    tft.vertScroll(0, (ILI9341_HEIGHT - KEYBOARD_HEIGHT), offset);
  }
};

unsigned int nextRefresh,bufferIndex;
CircularBuffer<char,LOCAL_BUFFER_SIZE> buffer;
void checkBuffer(){
  bool canviat = false;
  while(userTty->available() > 0) {
      buffer.push((char)userTty->read());
      canviat = true;
  }
  if(canviat)nextRefresh = millis() + latenciaPantalla;
}
void setup() {
  Serial1.begin(9600); // normal baud-rate

  userTty = &Serial1;// choose serial port here <------
  uint16_t tftID = tft.readID();
  tft.begin(tftID);
  
  input_init();

  tintty_run(
    [](){
      // peek idle loop
      while (true) {
        checkBuffer();
        if(nextRefresh < millis()){
          input_idle();
          if(!buffer.isEmpty()){
            return buffer.first();
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
          if(!buffer.isEmpty()){
            return buffer.shift();
          }else{
            tintty_idle(&ili9341_display);
          }
        }
        
      }
    },//send char
    [](char ch){
      checkBuffer();
      userTty->print(ch); 
      nextRefresh = millis() + latenciaPantalla;
    }
    ,
    &ili9341_display
  );
}

void loop() {
}

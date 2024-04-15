#include "Arduino.h"

#include <TFT_eSPI.h>       // Hardware-specific library
//#include <MCUFRIEND_kbv.h>
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4
extern TFT_eSPI tft;
//extern Adafruit_TFTLCD tft;
extern Stream *userTty;
// @todo move?
#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

#define KEYBOARD_HEIGHT 92

void input_init();
void input_idle();
void checkDoCalibration();

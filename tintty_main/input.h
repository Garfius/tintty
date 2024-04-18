#include "Arduino.h"

#include <TFT_eSPI.h>       // Hardware-specific library
//#include <MCUFRIEND_kbv.h>
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library
/*
struct bufferManager {
    bool isNew;
    bool isDrawing;
    u16_t minX,minY,maxY,maxY;
    TFT_eSprite fullPicture = TFT_eSprite(&tft); // Sprite object graph1
  Push a windowed area of the sprite to the TFT at tx, ty
  bool     pushSprite(int32_t tx, int32_t ty, int32_t sx, int32_t sy, int32_t sw, int32_t sh);

void     pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h, const uint16_t *data);
pushToSprite
*/

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4
extern TFT_eSPI tft;
//extern Adafruit_TFTLCD tft;
extern Stream *userTty;
#define errorLed 6

extern void giveErrorVisibility(bool init);
// @todo move?
#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

#define KEYBOARD_HEIGHT 92

void input_init();
void input_idle();
void checkDoCalibration();

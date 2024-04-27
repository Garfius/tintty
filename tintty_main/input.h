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
#define errorLed 6
// @todo move?
#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320
#define KEYBOARD_HEIGHT 92
#define keyboardAutoRepeatMillis 750

extern TFT_eSPI tft;// AQUI TENS ELS DOS -- SPRITE NEW
extern TFT_eSprite spr;
extern Stream *userTty;


extern void giveErrorVisibility(bool init);
void input_init();
void input_idle();
void checkDoCalibration();

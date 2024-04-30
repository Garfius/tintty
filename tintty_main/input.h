#include "Arduino.h"

#include <TFT_eSPI.h>       // Hardware-specific library
#define errorLed 6

// @todo move?
/*
#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320
*/
#define ILI9341_WIDTH TFT_WIDTH
#define ILI9341_HEIGHT TFT_HEIGHT
#define KEYBOARD_HEIGHT 92

#define keyboardAutoRepeatMillis 750
#define keyboardReleaseMillis 750

extern TFT_eSPI tft;// AQUI TENS ELS DOS -- SPRITE NEW
extern TFT_eSprite spr;
extern Stream *userTty;
extern void giveErrorVisibility(bool init);

void input_init();
void input_idle();
void checkDoCalibration();

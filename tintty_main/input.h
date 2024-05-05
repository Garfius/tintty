#include "Arduino.h"

#include <TFT_eSPI.h>// Hardware-specific library, configure at User_Setup.h

#define errorLed 6 // <-- to giveErrorVisibility at input.cpp

#define ILI9341_WIDTH TFT_WIDTH
#define ILI9341_HEIGHT TFT_HEIGHT

#define KEYBOARD_GUTTER 4

#define KEY_WIDTH (16+6)
#define KEY_HEIGHT (16+4)
#define KEY_GUTTER 1
#define KEYBOARD_HEIGHT ((6 * KEY_GUTTER)+(5*KEY_HEIGHT)+KEYBOARD_GUTTER)

#define KEY_ROW_A_Y (ILI9341_HEIGHT - KEYBOARD_HEIGHT + KEYBOARD_GUTTER)//480-(6 * 1)+(5*(16+3))+4 +4

#define keyboardAutoRepeatMillis 750
#define keyboardReleaseMillis 750

extern TFT_eSPI tft;
extern TFT_eSprite spr;
extern Stream *userTty;
extern void giveErrorVisibility(bool init);

void input_init();
void input_idle();

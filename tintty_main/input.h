#include "Arduino.h"
#include "config.h"
#include <TFT_eSPI.h>// Hardware-specific library, configure at User_Setup.h

#define KEYBOARD_GUTTER 4

#define KEY_WIDTH (16+6)
#define KEY_HEIGHT (16+4)
#define KEY_GUTTER 1
#define KEYBOARD_HEIGHT ((6 * KEY_GUTTER)+(5*KEY_HEIGHT)+KEYBOARD_GUTTER)

#define KEY_ROW_A_Y (TFT_ALSSADA - KEYBOARD_HEIGHT + KEYBOARD_GUTTER)//480-(6 * 1)+(5*(16+3))+4 +4


extern TFT_eSPI tft;
extern TFT_eSprite spr;
extern Stream *userTty;

void input_init();
void input_idle();

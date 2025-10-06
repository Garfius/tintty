/**
 * ST7735 rows 16
 * ST7735 Cols 26
 * ILI9488 rows 24
 * ILI9488 Cols 80
 */
//----------de main.cpp

#define snappyMillisLimit 185 // idle refresh time
#define tintty_baud_rate 57600
#define refreshMillisLimit 2000
#define TOUCH_IRQ 8

#define TOUCH_SENSIVITY 600

// de utils.h
#define INPUT_BUFFER_SIZE 25
#define OUTPUT_BUFFER_SIZE 25

// de input.h
#define errorLed 6 // <-- to giveErrorVisibility at input.cpp

#define keyboardAutoRepeatMillis 250
#define keyboardReleaseMillis 75// anti bounce

#define TFT_AMPLADA TFT_WIDTH
#define TFT_ALSSADA TFT_HEIGHT

// de input.cpp


// de tintty.h

// de tintty.cpp
#define cursorBlinkDelay 650

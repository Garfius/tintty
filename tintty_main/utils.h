#include <Arduino.h>
#ifndef __tinttyUtils__
#define __tinttyUtils__

#include <input.h>
#include "config.h"

//-------------coses baud menu
#define NUM_OPTIONS_BAUD_MENU 4
#define SQUARE_W_BAUD_MENU (TFT_AMPLADA - 60)      // wide but with margin
#define SQUARE_H_BAUD_MENU 60                   // fixed height
#define GAP_Y_BAUD_MENU 20                      // space between squares

#define TOTAL_HEIGHT (NUM_OPTIONS_BAUD_MENU * SQUARE_H_BAUD_MENU + (NUM_OPTIONS_BAUD_MENU - 1) * GAP_Y_BAUD_MENU)

#define START_Y_BAUD_MENU ((TFT_ALSSADA - TOTAL_HEIGHT) / 2)
#define START_X_BAUD_MENU ((TFT_AMPLADA - SQUARE_W_BAUD_MENU) / 2)
#define DELAY_CONFIRM_BAUD_MENU 1500
//-------------altres
volatile static char myCharBuffer[INPUT_BUFFER_SIZE];// whole ram must be buffer, lol
volatile static char myCharBuffer2[OUTPUT_BUFFER_SIZE];// whole ram must be buffer, lol

class CharBuffer{
    public:
    volatile int head = 0;
    volatile int tail = 0;
    unsigned int localBufferSize;
    volatile char* myCharBuffer;

    CharBuffer(unsigned int size,volatile char* bufferArray);
    void addChar(char c);
    char consumeChar();
};
void tft_espi_calibrate_touch();
unsigned long chooseBauds();
void giveErrorVisibility(bool init);

extern CharBuffer buffer;
extern CharBuffer bufferoUT;
#endif
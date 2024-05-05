
#include "Arduino.h"
#include "input.h"
#define TINTTY_CHAR_WIDTH (5+1)// see .setFreeFont(GLCD); at main
#define TINTTY_CHAR_HEIGHT (7+1)

/**
 * Frame buffer(TFT_eSprite) state control
*/
struct fameBufferControl {
    uint16_t minX,maxX,minY,maxY;
    volatile bool outputting;
    volatile bool hasChanges;
    volatile unsigned int lastRemoteDataTime;
};
extern fameBufferControl myCheesyFB;
extern void assureRefreshArea(int16_t x, int16_t y, int16_t w, int16_t h);

extern bool tintty_cursor_key_mode_application;
/**
 * Renderer callbacks.
 */
struct tintty_display {
    int16_t screen_width, screen_height;
    int16_t screen_col_count, screen_row_count; // width and height divided by char size

    void (*fill_rect)(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);//tft.fillRect(x, y, w, h, color);
    void (*draw_pixels)(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels);
    void (*set_vscroll)(int16_t offset); // scroll offset for entire screen
};
/**
 * Main entry point.
 * Peek/read callbacks are expected to block until input is available;
 * while sketch is waiting for input, it should call the tintty_idle() hook
 * to allow animating cursor, etc.
 */
void tintty_run(
    char (*peek_char)(),
    char (*read_char)(),
    void (*send_char)(char str),
    tintty_display *display
);

/**
 * Hook to call while e.g. sketch is waiting for input
 */
void tintty_idle(
    tintty_display *display
);

static const uint16_t myPalette[] PROGMEM = {// CUSTOMIZE !
  TFT_BLACK,    //  0  ^-
  TFT_RED,      //  1  |-
  TFT_GREEN,    //  2  |-
  TFT_YELLOW,   //  3  |-
  TFT_BLUE,     //  4  |-
  TFT_MAGENTA,  //  5  |-
  TFT_CYAN,     //  6  |-
  TFT_WHITE,    //  7  |-
  TFT_DARKGREY, //  8  |Bold?
  TFT_PURPLE,   //  9  |Bold?
  TFT_NAVY,     // 10  |Bold?
  TFT_BROWN,    // 11  |Bold?
  TFT_ORANGE,   // 12  |Bold?
  TFT_MAROON,   // 13  |Bold?
  TFT_DARKGREEN,// 14  |Bold?
  TFT_PINK      // 15  |Bold?
};

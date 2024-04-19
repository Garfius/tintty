
#include "Arduino.h"
#include "input.h"
#define TINTTY_CHAR_WIDTH (5+1)
#define TINTTY_CHAR_HEIGHT (7+1)

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

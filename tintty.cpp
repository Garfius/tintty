// @todo move out direct TFT references
#include <TFT_ILI9163C.h>
#define TFT_BLACK   0x0000
#define TFT_BLUE    0x001F
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW  0xFFE0
#define TFT_WHITE   0xFFFF

#include "tintty.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8

const int16_t max_col = SCREEN_WIDTH / CHAR_WIDTH;
const int16_t max_row = SCREEN_HEIGHT / CHAR_HEIGHT;

const int16_t IDLE_CYCLE_MAX = 6000;
const int16_t IDLE_CYCLE_ON = 3000;

struct tintty_state {
    int16_t cursor_col, cursor_row;
    uint16_t bg_tft_color, fg_tft_color;

    // @todo deal with integer overflow
    int16_t top_row; // first displayed row in a logical scrollback buffer

    char out_char;
    int16_t out_char_col, out_char_row;

    int16_t idle_cycle_count; // @todo track during blocking reads mid-command
} state;

struct tintty_rendered {
    int16_t cursor_col, cursor_row;
    int16_t top_row;
} rendered;

void _render(TFT_ILI9163C *tft) {
    // if scrolling, prepare the "recycled" screen area
    if (state.top_row != rendered.top_row) {
        // clear the new piece of screen to be recycled as blank space
        int16_t row_delta = state.top_row - rendered.top_row;

        // @todo handle scroll-up
        if (row_delta > 0) {
            // pre-clear the lines at the bottom
            int16_t clear_height = min(SCREEN_HEIGHT, row_delta * CHAR_HEIGHT);
            int16_t clear_sbuf_bottom = (state.top_row * CHAR_HEIGHT + SCREEN_HEIGHT) % SCREEN_HEIGHT;
            int16_t clear_sbuf_top = clear_sbuf_bottom - clear_height;

            // if rectangle straddles the screen buffer top edge, render that slice at bottom edge
            if (clear_sbuf_top < 0) {
                tft->fillRect(
                    0,
                    clear_sbuf_top + SCREEN_HEIGHT,
                    SCREEN_WIDTH,
                    -clear_sbuf_top,
                    state.bg_tft_color
                );
            }

            // if rectangle is not entirely above top edge, render the normal slice
            if (clear_sbuf_bottom > 0) {
                tft->fillRect(
                    0,
                    max(0, clear_sbuf_top),
                    SCREEN_WIDTH,
                    clear_sbuf_bottom - max(0, clear_sbuf_top),
                    state.bg_tft_color
                );
            }
        }

        // update displayed scroll
        tft->scroll((state.top_row * CHAR_HEIGHT) % SCREEN_HEIGHT);

        // save rendered state
        rendered.top_row = state.top_row;
    }

    // render character if needed
    if (state.out_char != 0) {
        tft->drawChar(
            state.out_char_col * CHAR_WIDTH,
            (state.out_char_row * CHAR_HEIGHT) % SCREEN_HEIGHT,
            state.out_char,
            state.fg_tft_color,
            state.bg_tft_color,
            1
        );

        // clear for next render
        state.out_char = 0;

        // the char draw may overpaint the cursor, in which case
        // mark it for repaint
        if (
            rendered.cursor_col == state.cursor_col &&
            rendered.cursor_row == state.cursor_row
        ) {
            rendered.cursor_col = -1;
            rendered.cursor_row = -1;
        }
    }

    // always redraw cursor to animate
    if (
        rendered.cursor_col != state.cursor_col ||
        rendered.cursor_row != state.cursor_row
    ) {
        // @todo clear old cursor unless it was not shown
    }

    // reflect new cursor position on screen
    tft->fillRect(
        state.cursor_col * CHAR_WIDTH,
        (state.cursor_row * CHAR_HEIGHT + CHAR_HEIGHT - 1) % SCREEN_HEIGHT,
        CHAR_WIDTH,
        1,
        state.idle_cycle_count < IDLE_CYCLE_ON
            ? state.fg_tft_color
            : state.bg_tft_color
    );

    // save new rendered state
    rendered.cursor_col = state.cursor_col;
    rendered.cursor_row = state.cursor_row;
}

void _main(
    char (*read_char)(),
    void (*send_char)(char str),
    TFT_ILI9163C *tft
) {
    // start in default idle state
    char initial_character = read_char();

    if (initial_character >= 0x20 && initial_character <= 0x7e) {
        // output displayable character
        state.out_char = initial_character;
        state.out_char_col = state.cursor_col;
        state.out_char_row = state.cursor_row;

        // update caret
        state.cursor_col += 1;

        if (state.cursor_col >= max_col) {
            state.cursor_col = 0;
            state.cursor_row += 1;

            // move displayed window down to cover cursor
            if (state.cursor_row >= max_row) {
                state.top_row = state.cursor_row - max_row + 1;
            }
        }

        // reset idle state
        state.idle_cycle_count = 0;

        _render(tft);
    } else {
        state.idle_cycle_count = (state.idle_cycle_count + 1) % IDLE_CYCLE_MAX;

        _render(tft);
    }
}

void tintty_run(
    char (*read_char)(),
    void (*send_char)(char str),
    TFT_ILI9163C *tft
) {
    // set up initial state
    state.cursor_col = 0;
    state.cursor_row = 0;
    state.top_row = 0;
    state.fg_tft_color = TFT_WHITE;
    state.bg_tft_color = TFT_BLACK;

    state.out_char = 0;

    rendered.cursor_col = -1;
    rendered.cursor_row = -1;

    // set up fullscreen TFT scroll
    // magic value for second parameter (bottom-fixed-area)
    // compensate for extra length of graphical RAM (GRAM)
    tft->defineScrollArea(0, 32);

    // initial render
    _render(tft);

    // main read cycle
    while (1) {
        _main(read_char, send_char, tft);
    }
}
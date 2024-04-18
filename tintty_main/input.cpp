#include "Arduino.h"
#include <EEPROM.h>

#include "tintty.h"
#include "input.h"

// calibrated settings from TouchScreen_Calibr_native
//const int XP=6,XM=A2,YP=A1,YM=7; //240x320 ID=0x9341 leonardo
int TS_LEFT=203,TS_RT=911,TS_TOP=213,TS_BOT=944;
/*
// using stock MCUFRIEND 2.4inch shield


#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define YP A1  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XM A2  // must be an analog pin, use "An" notation!
#define XP 6   // can be a digital pin

#define TS_MINX 203
#define TS_MINY 213
#define TS_MAXX 911
#define TS_MAXY 944
*/

#define TOUCH_TRIGGER_COUNT 20

#define KEYBOARD_GUTTER 4

#define KEY_WIDTH 16
#define KEY_HEIGHT 16
#define KEY_GUTTER 1

#define KEY_ROW_A_Y (ILI9341_HEIGHT - KEYBOARD_HEIGHT + KEYBOARD_GUTTER)

#define KEY_ROW_A_X(index) (0 + (KEY_WIDTH + KEY_GUTTER) * index)
#define KEY_ROW_B_X(index) (20 + (KEY_WIDTH + KEY_GUTTER) * index)
#define KEY_ROW_C_X(index) (24 + (KEY_WIDTH + KEY_GUTTER) * index)
#define KEY_ROW_D_X(index) (32 + (KEY_WIDTH + KEY_GUTTER) * index)
#define ARROW_KEY_X(index) (ILI9341_WIDTH - (KEY_WIDTH + KEY_GUTTER) * (4 - index))


//------------------ memory leak, mapped to char
#define KEYCODE_SHIFT -20
#define KEYCODE_CAPS -21
#define KEYCODE_CONTROL -22
#define KEYCODE_ARROW_START -30 // from -30 to -27

const int touchKeyRowCount = 5;

struct touchKey {
    int16_t x, width;
    int code, shiftCode; // <-- enten com va això i ho tens arreglat, busca per les fletxes, linia 305,392,390?
    char label;
};

struct touchKeyRow {
    int16_t y;

    int keyCount;
    struct touchKey keys[14];
} touchKeyRows[5] = {
    {
        KEY_ROW_A_Y,
        14,
        {
            { 1, KEY_ROW_A_X(1) - 1 - KEY_GUTTER, '\e', '~', 0 }, // shrunk width
            { KEY_ROW_A_X(1), KEY_WIDTH, '1', '!', 0 },
            { KEY_ROW_A_X(2), KEY_WIDTH, '2', '@', 0 },
            { KEY_ROW_A_X(3), KEY_WIDTH, '3', '#', 0 },
            { KEY_ROW_A_X(4), KEY_WIDTH, '4', '$', 0 },
            { KEY_ROW_A_X(5), KEY_WIDTH, '5', '%', 0 },
            { KEY_ROW_A_X(6), KEY_WIDTH, '6', '^', 0 },
            { KEY_ROW_A_X(7), KEY_WIDTH, '7', '&', 0 },
            { KEY_ROW_A_X(8), KEY_WIDTH, '8', '*', 0 },
            { KEY_ROW_A_X(9), KEY_WIDTH, '9', '(', 0 },
            { KEY_ROW_A_X(10), KEY_WIDTH, '0', ')', 0 },
            { KEY_ROW_A_X(11), KEY_WIDTH, '-', '_', 0 },
            { KEY_ROW_A_X(12), KEY_WIDTH, '=', '+', 0 },
            { KEY_ROW_A_X(13), ILI9341_WIDTH - 1 - KEY_ROW_A_X(13), 8, 8, 27 }
        }
    },
    {
        KEY_ROW_A_Y + (KEY_GUTTER + KEY_HEIGHT) * 1,
        14,
        {
            { 1, (KEY_ROW_B_X(0) - KEY_GUTTER - 1), 9, 9, 26 },

            { KEY_ROW_B_X(0), KEY_WIDTH, 'q', 'Q', 0 },
            { KEY_ROW_B_X(1), KEY_WIDTH, 'w', 'W', 0 },
            { KEY_ROW_B_X(2), KEY_WIDTH, 'e', 'E', 0 },
            { KEY_ROW_B_X(3), KEY_WIDTH, 'r', 'R', 0 },
            { KEY_ROW_B_X(4), KEY_WIDTH, 't', 'T', 0 },
            { KEY_ROW_B_X(5), KEY_WIDTH, 'y', 'Y', 0 },
            { KEY_ROW_B_X(6), KEY_WIDTH, 'u', 'U', 0 },
            { KEY_ROW_B_X(7), KEY_WIDTH, 'i', 'I', 0 },
            { KEY_ROW_B_X(8), KEY_WIDTH, 'o', 'O', 0 },
            { KEY_ROW_B_X(9), KEY_WIDTH, 'p', 'P', 0 },
            { KEY_ROW_B_X(10), KEY_WIDTH, '[', '{', 0 },
            { KEY_ROW_B_X(11), KEY_WIDTH, ']', '}', 0 },
            { KEY_ROW_B_X(12), ILI9341_WIDTH - 1 - KEY_ROW_B_X(12), '\\', '|', 0 }
        }
    },
    {
        KEY_ROW_A_Y + (KEY_GUTTER + KEY_HEIGHT) * 2,
        13,
        {
            {
                1,
                (KEY_ROW_C_X(0) - KEY_GUTTER - 1),
                KEYCODE_CAPS,
                KEYCODE_CAPS,
                18
            },

            { KEY_ROW_C_X(0), KEY_WIDTH, 'a', 'A', 0 },
            { KEY_ROW_C_X(1), KEY_WIDTH, 's', 'S', 0 },
            { KEY_ROW_C_X(2), KEY_WIDTH, 'd', 'D', 0 },
            { KEY_ROW_C_X(3), KEY_WIDTH, 'f', 'F', 0 },
            { KEY_ROW_C_X(4), KEY_WIDTH, 'g', 'G', 0 },
            { KEY_ROW_C_X(5), KEY_WIDTH, 'h', 'H', 0 },
            { KEY_ROW_C_X(6), KEY_WIDTH, 'j', 'J', 0 },
            { KEY_ROW_C_X(7), KEY_WIDTH, 'k', 'K', 0 },
            { KEY_ROW_C_X(8), KEY_WIDTH, 'l', 'L', 0 },
            { KEY_ROW_C_X(9), KEY_WIDTH, ';', ':', 0 },
            { KEY_ROW_C_X(10), KEY_WIDTH, '\'', '"', 0 },

            { KEY_ROW_C_X(11), ILI9341_WIDTH - 1 - KEY_ROW_C_X(11), 13, 13, 16 }
        }
    },
    {
        KEY_ROW_A_Y + (KEY_GUTTER + KEY_HEIGHT) * 3,
        12,
        {
            { KEY_ROW_D_X(0), KEY_WIDTH, 'z', 'Z', 0 },
            { KEY_ROW_D_X(1), KEY_WIDTH, 'x', 'X', 0 },
            { KEY_ROW_D_X(2), KEY_WIDTH, 'c', 'C', 0 },
            { KEY_ROW_D_X(3), KEY_WIDTH, 'v', 'V', 0 },
            { KEY_ROW_D_X(4), KEY_WIDTH, 'b', 'B', 0 },
            { KEY_ROW_D_X(5), KEY_WIDTH, 'n', 'N', 0 },
            { KEY_ROW_D_X(6), KEY_WIDTH, 'm', 'M', 0 },
            { KEY_ROW_D_X(7), KEY_WIDTH, ',', '<', 0 },
            { KEY_ROW_D_X(8), KEY_WIDTH, '.', '>', 0 },
            { KEY_ROW_D_X(9), KEY_WIDTH, '/', '?', 0 },

            {
                1,
                (KEY_ROW_D_X(0) - KEY_GUTTER - 1),
                KEYCODE_SHIFT,
                KEYCODE_SHIFT,
                24
            },

            {
                KEY_ROW_D_X(10),
                ILI9341_WIDTH - 1 - KEY_ROW_D_X(10),
                KEYCODE_SHIFT,
                KEYCODE_SHIFT,
                24
            }
        }
    },
    {
        KEY_ROW_A_Y + (KEY_GUTTER + KEY_HEIGHT) * 4,
        6,
        {
            {
                1,
                22,
                KEYCODE_CONTROL,
                KEYCODE_CONTROL,
                'C'
            },

            { (ILI9341_WIDTH - 100) / 2, 100, ' ', ' ', ' ' },

            { ARROW_KEY_X(0), KEY_WIDTH, KEYCODE_ARROW_START + 3, KEYCODE_ARROW_START + 3, 17 },
            { ARROW_KEY_X(1), KEY_WIDTH, KEYCODE_ARROW_START, KEYCODE_ARROW_START, 30 },
            { ARROW_KEY_X(2), KEY_WIDTH, KEYCODE_ARROW_START + 1, KEYCODE_ARROW_START + 1, 31 },
            { ARROW_KEY_X(3), KEY_WIDTH, KEYCODE_ARROW_START + 2, KEYCODE_ARROW_START + 2, 16 }
        }
    }
};

struct touchKeyRow *activeRow = NULL;
struct touchKey *activeKey = NULL;
bool shiftIsActive = false;
bool shiftIsSticky = false;
bool controlIsActive = false;

void _input_set_mode(bool shift, bool shiftSticky, bool control) {
    // reset if current mode already matches
    if (
        shiftIsActive == shift &&
        shiftIsSticky == shiftSticky &&
        controlIsActive == control
    ) {
        shiftIsActive = false;
        shiftIsSticky = false;
        controlIsActive = false;
    } else {
        shiftIsActive = shift;
        shiftIsSticky = shiftSticky;
        controlIsActive = control;
    }
}

void _input_draw_key(struct touchKeyRow *keyRow, struct touchKey *key) {
    const int16_t rowCY = keyRow->y;
    const bool isActive = (
        key == activeKey ||
        (shiftIsActive && (
            (key->code == KEYCODE_SHIFT && !shiftIsSticky) ||
            (key->code == KEYCODE_CAPS && shiftIsSticky)
        )) ||
        (controlIsActive && key->code == KEYCODE_CONTROL)
    );

    uint16_t keyColor = isActive ? 0xFFFF : 0;
    uint16_t borderColor = isActive ? 0xFFFF : tft.color565(0x80, 0x80, 0x80);
    uint16_t textColor = isActive ? 0 : 0xFFFF;

    tft.setTextColor(textColor);

    const int16_t ox = key->x;
    const int16_t oy = rowCY;

    tft.drawFastHLine(ox, oy, key->width, borderColor);
    tft.drawFastHLine(ox, oy + KEY_HEIGHT - 1, key->width, borderColor);
    tft.drawFastVLine(ox, oy, KEY_HEIGHT, borderColor);
    tft.drawFastVLine(ox + key->width - 1, oy, KEY_HEIGHT, borderColor);
    tft.fillRect(ox + 1, oy + 1, key->width - 2, KEY_HEIGHT - 2, keyColor);

    tft.setCursor(key->x + (key->width / 2) - 3, rowCY + (KEY_HEIGHT / 2) - 4);
    tft.print(
        key->label == 0
            ? (shiftIsActive ? (char)key->shiftCode : (char)key->code)
            : key->label
    );
}

void _input_draw_all_keys() {
    for (int i = 0; i < touchKeyRowCount; i++) {
        struct touchKeyRow *keyRow = &touchKeyRows[i];
        const int keyCount = keyRow->keyCount;

        for (int j = 0; j < keyCount; j++) {
            _input_draw_key(keyRow, &keyRow->keys[j]);
        }
    }
}

void _input_process_touch(int16_t xpos, int16_t ypos) {
    activeRow = NULL;

    for (int i = 0; i < touchKeyRowCount; i++) {
        int rowCY = touchKeyRows[i].y;

        if (ypos >= rowCY && ypos <= rowCY + KEY_HEIGHT) {
            activeRow = &touchKeyRows[i];
            break;
        }
    }

    if (activeRow == NULL) {
        return;
    }

    const int keyCount = activeRow->keyCount;

    for (int i = 0; i < keyCount; i++) {
        struct touchKey *key = &activeRow->keys[i];

        if (xpos < key->x || xpos > key->x + key->width) {
            continue;
        }

        // activate the key
        activeKey = key;
        break;
    }

    if (activeKey) {
        if (activeKey->code == KEYCODE_SHIFT) {
            _input_set_mode(true, false, false);
            _input_draw_all_keys();
        } else if (activeKey->code == KEYCODE_CAPS) {
            _input_set_mode(true, true, false);
            _input_draw_all_keys();
        } else if (activeKey->code == KEYCODE_CONTROL) {
            _input_set_mode(false, false, true);
            _input_draw_all_keys(); // redraw all, to clear previous mode
        } else {
            _input_draw_key(activeRow, activeKey);

            if (shiftIsActive) {
                userTty->print((char)activeKey->shiftCode);

                // clear back to lowercase unless caps-lock
                if (!shiftIsSticky) {
                    _input_set_mode(false, false, false);
                    _input_draw_all_keys();
                }
            } else if (controlIsActive) {
                if (activeKey->code >= 97 && activeKey->code <= 122) {
                    // alpha control keys
                    userTty->print((char)(activeKey->code - 96));
                } else if (activeKey->code >= 91 && activeKey->code <= 93) {
                    // [, / and ] control keys
                    // @todo other stragglers
                    userTty->print((char)(activeKey->code - 91 + 27));
                }

                // always clear back to normal
                _input_set_mode(false, false, false);
                _input_draw_all_keys();
            } else if (activeKey->code >= KEYCODE_ARROW_START && activeKey->code < KEYCODE_ARROW_START + 4) {
                userTty->print((char)27); // Esc
                userTty->print(tintty_cursor_key_mode_application ? 'O' : '['); // different code depending on terminal state
                userTty->print((char)(activeKey->code - KEYCODE_ARROW_START + 'A'));
            } else {
                userTty->print((char)activeKey->code);
            }
        }
    }
}

void _input_process_release() {
    struct touchKeyRow *releasedKeyRow = activeRow;
    struct touchKey *releasedKey = activeKey;

    activeRow = NULL;
    activeKey = NULL;

    _input_draw_key(releasedKeyRow, releasedKey);
}

void input_init(){
    //checkDoCalibration();
    
    uint16_t bgColor = tft.color565(0x20, 0x20, 0x20);

    tft.fillRect(0, ILI9341_HEIGHT - KEYBOARD_HEIGHT, ILI9341_WIDTH, KEYBOARD_HEIGHT, bgColor);

    tft.setTextSize(1);

    _input_draw_all_keys();
}
#define keyboardAutoRepeatMillis 200
uint16_t xpos,ypos;
bool isTouching = false;
unsigned int nextPush = 0;
void input_idle() {
    if (tft.getTouch(&xpos, &ypos)) {
        if(!isTouching){
            nextPush = millis()+keyboardAutoRepeatMillis;
            isTouching = true;
            _input_process_touch(xpos, ypos);// <-- empalme!!
        }else{
            if(millis() > nextPush){
                _input_process_release();// <-- empalme!!
                _input_process_touch(xpos, ypos);// <-- empalme!!
            }
        }
    }else{
        isTouching = false;
        _input_process_release();// <-- empalme!!
    }
}

void writeUint16_t(int address, uint16_t value) {
  // Write the high byte (most significant byte) at the address
  EEPROM.write(address, value >> 8);
  // Write the low byte (least significant byte) at the next address
  EEPROM.write(address + 1, value & 0xFF);
}

uint16_t readUint16_t(int address) {
  // Read the high byte (most significant byte) from the address
  uint16_t value = EEPROM.read(address) << 8;
  // Read the low byte (least significant byte) from the next address
  value |= EEPROM.read(address + 1);
  return value;
}
void giveErrorVisibility(bool init){
    if(init){
        pinMode(errorLed,OUTPUT);
        digitalWrite(errorLed,LOW);
        delay(250);
        digitalWrite(errorLed,HIGH);
        delay(250);
        digitalWrite(errorLed,LOW);
    }else{
        digitalWrite(errorLed,HIGH);
    }
}
/**
 * int TS_LEFT,TS_RT,TS_TOP,TS_BOT;
void checkDoCalibration(){
    int16_t pointsX[4];
    int16_t pointsY[4];
    tp = ts.getPoint();
    if(!(tp.z > MINPRESSURE && tp.z < MAXPRESSURE)){// do calibrate    
        // load from eeprom
        return;
    }
    userTty->println("calibrate");

    bool pointDone = false;
    tft.fillCircle(20, 20,3, 0xF800);
    while(!pointDone){
        tp = ts.getPoint();
        if(tp.z > MINPRESSURE && tp.z < MAXPRESSURE){// do calibrate
            pointsX[0] = tp.x;
            pointsY[0] = tp.y;
            pointDone = true;
        }
    }
    pointDone = false;
    tft.fillCircle(220, 20,3, 0xF800);
    while(!pointDone){
        tp = ts.getPoint();
        if(tp.z > MINPRESSURE && tp.z < MAXPRESSURE){// do calibrate
            pointsX[1] = tp.x;
            pointsY[1] = tp.y;
            pointDone = true;
        }
    }
    pointDone = false;
    tft.fillCircle(220, 300,3, 0xF800);
    while(!pointDone){
        tp = ts.getPoint();
        if(tp.z > MINPRESSURE && tp.z < MAXPRESSURE){// do calibrate
            pointsX[2] = tp.x;
            pointsY[2] = tp.y;
            pointDone = true;
        }
    }
    pointDone = false;
    tft.fillCircle(20, 300, 3, 0xF800);
    while(!pointDone){
        tp = ts.getPoint();
        if(tp.z > MINPRESSURE && tp.z < MAXPRESSURE){// do calibrate
            pointsX[3] = tp.x;
            pointsY[3] = tp.y;
            pointDone = true;
        }
    }

    // maxims son a TS_RT i TS_BOT
    for(int i=0;i< 4;i++){
        userTty->print("i=");
        userTty->println(i);
        userTty->print("x=");
        userTty->println(pointsX[i]);
        userTty->print("y=");
        userTty->println(pointsX[i]);
    }

}
*/
Stream *userTty;
//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

TFT_eSPI tft = TFT_eSPI();

//MCUFRIEND_kbv tft;
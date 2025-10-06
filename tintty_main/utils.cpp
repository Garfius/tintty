#include <utils.h>
#include <EEPROM.h>

/**
 * NOT optimized AT ALL! just debugged
 */
void tft_espi_calibrate_touch()
{
  uint16_t calibrationData[7]; // els 2 extra son per fer el check de si es vol fer a mà
  uint8_t *calibrationDataBytePoint = (uint8_t *)calibrationData;
  uint8_t calDataOK = 0;
  uint8_t calDataTst = 0;
  //-------------------eeprom integrity check
  calDataOK = EEPROM.read(0);
  calDataTst = ~EEPROM.read(1);
  calDataOK = calDataOK == calDataTst;
  int eePos = 2;
  if (calDataOK && (!tft.getTouch(&calibrationData[5], &calibrationData[6],TOUCH_SENSIVITY)))
  {
    // calibration data valid & NO TSOLICITED
    for (int i = 0; i < 10; i++)
    {
      calDataTst = EEPROM.read(eePos + i);
      calibrationDataBytePoint[i] = calDataTst;
      eePos++;
    }
    tft.setTouch(calibrationData);
  }
  else
  {
    if (tft.getTouch(&calibrationData[5], &calibrationData[6],TOUCH_SENSIVITY))
    {
      tft.fillScreen(TFT_YELLOW);
      delay(1000);
      tft.fillScreen(TFT_BLACK);
    }
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    while (calDataOK == EEPROM.read(0))
      calDataOK = (uint8_t)random(0, 255); // firma
    EEPROM.write(0, calDataOK);
    calDataTst = ~calDataOK;
    EEPROM.write(1, calDataTst);
    int eePos = 2;
    for (int i = 0; i < 10; i++)
    {
      calDataTst = calibrationDataBytePoint[i];
      EEPROM.write(eePos + i, calDataTst);
      eePos++;
    }
    EEPROM.commit();
    EEPROM.end();
  }
  tft.fillScreen(TFT_BLACK);
}

// Add a character to the buffer
CharBuffer::CharBuffer(unsigned int size, volatile char *bufferArray)
{
  myCharBuffer = bufferArray;
  localBufferSize = size;
}
void CharBuffer::addChar(char c)
{
  if (((tail + 1) % localBufferSize) == head)
  { // Buffer is full
    giveErrorVisibility(false);
  }
  myCharBuffer[tail] = c;
  tail = (tail + 1) % localBufferSize;
}

// Consume a character from the buffer
char CharBuffer::consumeChar()
{
  if (head == tail)
  { // Buffer is empty
    giveErrorVisibility(false);
  }
  char c = myCharBuffer[head];
  head = (head + 1) % localBufferSize;
  return c;
}


const unsigned long optionValues[NUM_OPTIONS_BAUD_MENU]= {9600, 19200,57600, 115200};
const char *optionLabels[NUM_OPTIONS_BAUD_MENU]= {"9600", "19200","57600", "115200"};

// ==== Function to draw big named squares ====
unsigned long chooseBauds(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0,0);
  tft.println("Escull Velocitat");
  for (int i = 0; i < NUM_OPTIONS_BAUD_MENU; i++) {
    int x = START_X_BAUD_MENU;
    int y = START_Y_BAUD_MENU + i * (SQUARE_H_BAUD_MENU + GAP_Y_BAUD_MENU);

    // Draw outlined square (rectangle)
    tft.drawRect(x, y, SQUARE_W_BAUD_MENU, SQUARE_H_BAUD_MENU, TFT_WHITE);

    // Center text
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(x + (SQUARE_W_BAUD_MENU) / 2, y + (SQUARE_H_BAUD_MENU) / 2);
    tft.println(optionLabels[i]);
  }
  
  bool escullBe = false;
  uint16_t xpos,ypos;
  unsigned long tmp;
  while(!escullBe){
    while(!digitalRead(TOUCH_IRQ));
    if (tft.getTouch(&xpos, &ypos,TOUCH_SENSIVITY)) {
      for (int i = 0; i < NUM_OPTIONS_BAUD_MENU; i++) {
        int x = START_X_BAUD_MENU;
        int y = START_Y_BAUD_MENU + i * (SQUARE_H_BAUD_MENU + GAP_Y_BAUD_MENU);

        if (xpos >= x && xpos <= (x + SQUARE_W_BAUD_MENU) &&
            ypos >= y && ypos <= (y + SQUARE_H_BAUD_MENU)) 
        {
            tmp = optionValues[i]; // Found the option
            escullBe = true;
            tft.fillRect(x, y, SQUARE_W_BAUD_MENU, SQUARE_H_BAUD_MENU, TFT_WHITE);
            tft.setTextColor(TFT_BLACK,TFT_WHITE);
            tft.setCursor(x + (SQUARE_W_BAUD_MENU) / 2, y + (SQUARE_H_BAUD_MENU ) / 2);
            tft.println(optionLabels[i]);
            delay(DELAY_CONFIRM_BAUD_MENU);
            break;
        }
      }
    }
  }
  return tmp;
}

void giveErrorVisibility(bool init)
{
  if (init)
  { // breakpoint here
    pinMode(errorLed, OUTPUT);
    digitalWrite(errorLed, LOW);
    delay(250);
    digitalWrite(errorLed, HIGH);
    delay(250);
    digitalWrite(errorLed, LOW);
  }
  else
  {
    digitalWrite(errorLed, HIGH);
  }
}

CharBuffer buffer = CharBuffer(INPUT_BUFFER_SIZE, myCharBuffer);
CharBuffer bufferoUT = CharBuffer(OUTPUT_BUFFER_SIZE, myCharBuffer2);

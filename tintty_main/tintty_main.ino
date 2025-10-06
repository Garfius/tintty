#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch from TFT_eSPI
#include "input.h"
#include "tintty.h"
#include "utils.h"
#include "config.h"
/**
 * TinTTY main sketch
 * by Nick Matantsev 2017 & Gerard Forcada 2024
 *
 * Original reference: VT100 emulation code written by Martin K. Schroeder
 * and modified by Peter Scargill.
 *
 * to-do:
 *  Test on the original ILI9341
 *  Port back to AVR if possible
 *  Scroll back
 *  Improve multi-core, use myCheesyFB.outputting so refresh while receiving
 *
 */

volatile bool running = false;

tintty_display ili9341_display = {					 // from serial to display from ~236 tintty_idle(&ili9341_display)
	TFT_AMPLADA,											 // x
	(TFT_ALSSADA - KEYBOARD_HEIGHT),						 // y
	TFT_AMPLADA / TINTTY_CHAR_WIDTH,						 // colCount
	(TFT_ALSSADA - KEYBOARD_HEIGHT) / TINTTY_CHAR_HEIGHT, // rowCount

	[](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { // fill_rect, cursor
		assureRefreshArea(x, y, w, h);
		spr.fillRect(x, y, w, h, color);
	},

	[](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels) { // draw_pixels, no es fa servir
		assureRefreshArea(x, y, w, h);
		spr.setAddrWindow(x, y, x + w - 1, y + h - 1);
		spr.pushColors(pixels, w * h, 1);
	},

	[](int16_t offset) { // set_vscroll

	}};
// buffer to test various input sequences
// passa el tros indicat a;
//unsigned static int lastRefresh;
void refreshDisplayIfNeeded()
{
	// si hasChanges i myCheesyFB.lastRemoteDataTime < millis();  || (millis() > (myCheesyFB.lastRemoteDataTime+refreshMillisLimit))
	if (myCheesyFB.hasChanges)
	{
		//unsigned int tmp = millis();
		//if ((tmp > (myCheesyFB.lastRemoteDataTime + snappyMillisLimit)) || (tmp > (lastRefresh + refreshMillisLimit)))
    if (millis() > (myCheesyFB.lastRemoteDataTime + snappyMillisLimit))
		{
			// o esta ocupat pero fa estona
			myCheesyFB.outputting = true;
			spr.pushSprite(myCheesyFB.minX, myCheesyFB.minY, myCheesyFB.minX, myCheesyFB.minY, myCheesyFB.maxX - myCheesyFB.minX, myCheesyFB.maxY - myCheesyFB.minY);
			myCheesyFB.outputting = false;
			myCheesyFB.hasChanges = false;
			myCheesyFB = fameBufferControl{UINT16_MAX, 0, UINT16_MAX, 0, false, false, 0};
			//lastRefresh = tmp;
		}
	
  }
	else
	{
		input_idle(); // aqui colisiona mutex
	}
  
}
void parseToBuffer()
{

	tintty_run( // serial to Screen
		[]()
		{
			// peek idle loop, non blocking?
			while (true)
			{
				// bufferAtoB();

				// if (userTty->available() > 0)             return (char)userTty->peek(); // Safe to read

				if (buffer.head != buffer.tail)
					return myCharBuffer[buffer.head];
				tintty_idle(&ili9341_display); // render if needed

				// input_idle();// aqui colisiona mutex
			}
		},
		[]()
		{
			while (true)
			{ // read char
				// bufferAtoB();
				tintty_idle(&ili9341_display);
				// if (userTty->available() > 0) return (char)userTty->read(); // Safe to read
				if (buffer.head != buffer.tail)
					return buffer.consumeChar();

				// input_idle();// aqui colisiona mutex
			}
		}, // send char
		[](char ch)
		{
			bufferoUT.addChar(ch);
			// if (userTty->available() == 0) userTty->write(ch);
		},
		&ili9341_display);
}
void loop1()
{
	refreshDisplayIfNeeded();
	delay(1);
}
/*TaskHandle_t xHandle;
TaskHandle_t xHandle1;*/
void loop()
{
	/*xTaskCreate(vTaskReadSerial, "readSerial", 512, NULL, 1,  NULL );
	xTaskCreate(vTaskParseToBuffer, "parseToBuffer", 512, NULL, 1,  NULL );
	xTaskCreate(vTaskReadSerial, "readSerial", 512, NULL, 1,  &( xHandle ) );
	xTaskCreate(vTaskParseToBuffer, "parseToBuffer", 512, NULL, 1,  &( xHandle1 ) );
	vTaskCoreAffinitySet( xHandle, 1 << 0 );
	vTaskCoreAffinitySet( xHandle1, 1 << 0 );
	vTaskStartScheduler();
	*/
}

void setup1()
{
	while (!running)
	{
	}
}

void setup()
{
	//-----------------------setup
	pinMode(23, OUTPUT); // millora 3.3v GPIO23 controls the RT6150 PS (Power Save) pin. When PS is low (the default on Pico)
	digitalWrite(23, HIGH);
	Serial1.setFIFOSize(400);
	//Serial1.begin(tintty_baud_rate, SERIAL_8N1);
	EEPROM.begin(255);
	tft.begin();
	tft.setFreeFont(GLCD);

	gpio_pull_up(2); // ensure pull-up for receiving wire

	userTty = &Serial1; // assign receiving serial port

	//-----------------------init
	giveErrorVisibility(true);

	spr.setColorDepth(8);

	if (spr.createSprite(TFT_AMPLADA, (TFT_ALSSADA - KEYBOARD_HEIGHT)) == nullptr)
		giveErrorVisibility(false);

	spr.setTextSize(1);
	spr.fillSprite(TFT_BLACK);

	tft_espi_calibrate_touch();

	pinMode(TOUCH_IRQ, INPUT);
	
	Serial1.begin(chooseBauds(), SERIAL_8N1);
	
	tft.setTextSize(1);
	
	input_init();
	
	//---------------go!
	running = true;
	
	parseToBuffer();
}

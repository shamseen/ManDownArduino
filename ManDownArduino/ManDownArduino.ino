/*
 Name:		ManDownArduino.ino
 Created:	4/12/2017 1:19:08 PM
 Author:	Shamseen
*/

#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 4

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);


/**************************************************************************/
/*!
This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
	switch (event)
	{
	case ACI_EVT_DEVICE_STARTED:
		Serial.println(F("Advertising started"));
		break;
	case ACI_EVT_CONNECTED:
		Serial.println(F("Connected!"));
		break;
	case ACI_EVT_DISCONNECTED:
		Serial.println(F("Disconnected or advertising timed out"));
		break;
	default:
		break;
	}
}

/**************************************************************************/
/*!
This function is called whenever data arrives on the RX channel
Takes buffer and length of message
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{
	String msg = "bluetooth says: ";

	Serial.print(F("Received "));
	Serial.print(len);
	Serial.print(F(" bytes: "));
	for (int i = 0; i<len; i++)
		Serial.print((char)buffer[i]);

	Serial.print(F(" ["));

	for (int i = 0; i<len; i++)
	{
		Serial.print(" 0x");
		Serial.print((char)buffer[i], HEX);
		msg += (char)buffer[i];

	}
	Serial.println(F(" ]"));

	/* Echo the same data back! */
	len = msg.length();
	char msgArr[len];

	for (int i = 0; i<len; i++) {
		msgArr[i] = msg[i];
	}
	uart.write((uint8_t*)msgArr, len);
}

/**************************************************************************/
/*!
Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{
	Serial.begin(9600);
	while (!Serial); // Leonardo/Micro should wait for serial init
	Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Callback Echo demo"));

	uart.setRXcallback(rxCallback);
	uart.setACIcallback(aciCallback);
	uart.setDeviceName("ManDown"); /* 7 characters max! */
	uart.begin();
}

/**************************************************************************/
/*!
Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
	uart.pollACI();
}

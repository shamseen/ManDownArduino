/*
 Name:		ManDownArduino.ino
 Created:	4/12/2017 1:19:08 PM
 Author:	Shamseen and robertgp
*/

//*********************SHAMSEEN**********************************************
#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 4
Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

//*********************ROBERTO***********************************************
#include <Wire.h>
#include "Arduino.h"
#include "MMA7660.h"
MMA7660 accelemeter;
int8_t x;
int8_t y;
int8_t z;
float ax, ay, az;
const int table_length = 30;
int8_t pos[table_length][3];
float acce[table_length][3];
int row = 0;
int fall_count = 0;
int ignore_initial = 0;

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

bool detectFall() {
	accelemeter.getXYZ(&x, &y, &z);

	//Serial.print("x = ");
	//Serial.println(x); 
	//Serial.print("y = ");
	//Serial.println(y);   
	//Serial.print("z = ");
	//Serial.println(z);

	pos[row][0] = x;
	pos[row][1] = y;
	pos[row][2] = z;

	accelemeter.getAcceleration(&ax, &ay, &az);
	//Serial.println("accleration of X/Y/Z: ");
	//Serial.print(ax);
	//Serial.println(" g");
	//Serial.print(ay);
	//Serial.println(" g");
	//Serial.print(az);
	//Serial.println(" g");

	acce[row][0] = ax;
	acce[row][1] = ay;
	acce[row][2] = az;

	delay(500);

	Serial.println(pos[row][0]);
	Serial.println(pos[row][1]);
	Serial.println(pos[row][2]);

	Serial.println(acce[row][0]);
	Serial.println(acce[row][1]);
	Serial.println(acce[row][2]);

	// Determining a fall in this iteration
	if (ignore_initial >= 5) {
		if ((acce[row][0] <= 0.05 && acce[row][0] >= -0.05) || (acce[row][1] <= 0.05 && acce[row][1] >= -0.05) || (acce[row][2] <= 0.05 && acce[row][2] >= -0.05)) {
			Serial.println("Possible Fall, Wait for Confirmation");
			int8_t check_table[5][3];
			int d1x, d1y, d1z, d2x, d2y, d2z, d3x, d3y, d3z, d4x, d4y, d4z, dx, dy, dz;
			for (int i = 0; i < 5; i++) {
				accelemeter.getXYZ(&x, &y, &z);
				check_table[i][0] = x;
				check_table[i][1] = y;
				check_table[i][2] = z;
			}
			d1x = (check_table[1][0]) - (check_table[0][0]);
			d1y = (check_table[1][1]) - (check_table[0][1]);
			d1z = (check_table[1][2]) - (check_table[0][2]);

			d2x = (check_table[2][0]) - (check_table[1][0]);
			d2y = (check_table[2][1]) - (check_table[1][1]);
			d2z = (check_table[2][2]) - (check_table[1][2]);

			d3x = (check_table[3][0]) - (check_table[2][0]);
			d3y = (check_table[3][1]) - (check_table[2][1]);
			d3z = (check_table[3][2]) - (check_table[2][2]);

			d4x = (check_table[4][0]) - (check_table[3][0]);
			d4y = (check_table[4][1]) - (check_table[3][1]);
			d4z = (check_table[4][2]) - (check_table[3][2]);

			dx = d1x + d2x + d3x + d4x;
			dy = d1y + d2y + d3y + d4y;
			dz = d1z + d2z + d3z + d4z;

			if ((dx <= 10 && dx >= -10) && (dy <= 10 && dy >= -10) && (dz <= 10 && dz >= -10)) {
				fall_count++;
			}

			if (fall_count >= 3) {
				Serial.println("MAN DOWN");
				fall_count = 0;
				return true;
			}

		}
		else {
			fall_count = 0;

			// Incrememnting and "cleaning" the array 
			row++;

			Serial.println("*************");

			if (row == table_length) {
				Serial.println("Done with 1 table"); //Helps us know when one table has been filled
				row = 0;
			}
			delay(500);

			ignore_initial++;
		}
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
	for (int i = 0; i < len; i++)
		Serial.print((char)buffer[i]);

	Serial.print(F(" ["));

	for (int i = 0; i < len; i++)
	{
		Serial.print(" 0x");
		Serial.print((char)buffer[i], HEX);
		msg += (char)buffer[i];

	}
	Serial.println(F(" ]"));

	/* Echo the same data back! */
	len = msg.length();
	char msgArr[len];

	for (int i = 0; i < len; i++) {
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
	//*********************SHAMSEEN**********************************************
	Serial.begin(9600);
	while (!Serial); // Leonardo/Micro should wait for serial init
	Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Callback Echo demo"));

	uart.setRXcallback(rxCallback);
	uart.setACIcallback(aciCallback);
	uart.setDeviceName("ManDown"); /* 7 characters max! */
	uart.begin();

	//*********************ROBERTO***********************************************
	accelemeter.init();
}

/**************************************************************************/
/*!
Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
	uart.pollACI();

	bool isFall = detectFall();

	if (isFall)
	{
		//notify phone
	}
}


#include <Arduino.h>
#include <Wire.h>
// code for Reciever to print hello world from Michael
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define GND_PIN 1
#define Vcc_PIN 2
#define CE_PIN 3
#define CSN_PIN 4
#define SCLK_PIN 5
#define MOSI_PIN 6 // used to send?
#define MISO_PIN 7 // used to receive?

/*
Reciever To Do:
Need to recieve:
  - Gyroscope Data
  - Flex Sensor Data

Needed Pin Connections: 
(Based on chip datasheet from https://components101.com/wireless/nrf24l01-pinout-features-datasheet)
  - GND (Pin 1)
  - Vcc (Pin 2)
  - Chip Enable (Pin 3)
  - Chip Select Not (Pin 4) needs to be kept high
  - Serial Clock (Pin 5)
  - MOSI (Pin 6)
  - MISO (Pin 7)

IRQ (Pin 8) is not really needed, we are polling for data

Key operations:
Recieve Steering and Acceleration values

convert them to needed analog signals to our motors

*/
RF24 radio(CE_PIN, CSN_PIN); // CE, CSN

const byte address[6] = "00001";

/*
Message Structure:
only sending an integer
0 = Stationary
1 = Forward
2 = Left
3 = Right
4 = Error in msg
*/
void parseMessage(int message){

}

void setup() {
	Serial.begin(19200);
  
	// Set up radio into RX mode. may need to set up payload size
	radio.begin();
	radio.openReadingPipe(1, address);
	radio.setPALevel(RF24_PA_MIN);
	radio.startListening();
}

void loop() {
	if (radio.available()) {
        int text;
        radio.read(&text, sizeof(text));
        Serial.println(text);
    }
    //Serial.print("TEST");
}
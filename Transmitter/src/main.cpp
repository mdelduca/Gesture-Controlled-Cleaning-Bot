
#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*
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
*/


#define CE_PIN 7
#define CSN_PIN 8


/*
Transmitter To Do:
Need to send:
    - Steering Data
    - Acceleration Data

Steering Data:
    Use Gyroscope
    - set threshold for roll angle to determine whether we steer or not
    - send steering direction (not worrying about gradual angle steering, only one steering angle!)

Acceleration Data:
    Use Flex Sensor
    - more bending, higher resistance
    - ranges between 700 to 900, should be mapped to the 0 - 255 PWM range
    - we will need to convert the incoming 700-900 range into an intended speed value
    - convert this intended speed value into the PWM range for our motors
*/

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN

/*
Message Structure:
only sending an integer
0 = Stationary
1 = Forward
2 = Left
3 = Right
4 = Error in msg
*/


const byte address[6] = "00001";

void setup() 
{
	 Serial.begin(19200);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));
  Serial.println("message");
  delay(1000);
}

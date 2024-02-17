#include <Arduino.h>
#include <Wire.h>
// code for Transmitter to print hello world from Michael
#define led 5
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

#define GND_PIN 1
#define Vcc_PIN 2
#define CE_PIN 3
#define CSN_PIN 4
#define SCLK_PIN 5
#define MOSI_PIN 6 // used to send?
#define MISO_PIN 7 // used to receive?

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
void setup() {
    Serial.begin(9600);
    /*
    Wire.begin();                      // Initialize comunication
    Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
    Wire.write(0x6B);                  // Talk to the register 6B
    Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
    Wire.endTransmission(true);        //end the transmission
    */
    
    // setup radio into TX mode. may need to implement payload size
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();

}

int count = 0;
bool tx_ok, tx_fail, rx_ready;

void loop() {
    int msg = count;
	bool report = radio.write(&msg, sizeof(msg));
    radio.whatHappened(tx_ok, tx_fail, rx_ready);
	
    if(report){
        Serial.print(msg, DEC);
        Serial.print(" Sent! TX_OK, TX_FAIL, RX_READY");
		Serial.print(tx_ok, BIN);
		Serial.print(tx_fail, BIN);
		Serial.print(rx_ready, BIN);
		Serial.println();
    }
    count++;
    delay(1000);
    
}

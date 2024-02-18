
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
int count = 7;
void setup() {
    Serial.begin(19200);
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
    radio.setPALevel(RF24_PA_LOW);
    radio.stopListening();
	radio.setPayloadSize(sizeof(int));
	if(radio.failureDetected){
		Serial.println("FAILURE DETECTED!!! ERROR");
	}
}

int count = 0;
bool tx_ok, tx_fail, rx_ready;
int delayTime = 1000;

void loop() {
    int msg = count;
	bool report = radio.write(&msg, sizeof(msg));
    radio.whatHappened(tx_ok, tx_fail, rx_ready);

    if(report){
        Serial.println("Sent!");	
    }else{
		Serial.println("TX_OK, TX_FAIL, RX_READY, Count");
		Serial.println(tx_ok);
		Serial.println(tx_fail);
		Serial.println(rx_ready);
	}
	
	Serial.println(count);
    
	count++;
    delay(delayTime);
    
}

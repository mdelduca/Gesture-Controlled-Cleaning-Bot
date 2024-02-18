#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN 7
#define CSN_PIN 8


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

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

int delayTime = 500;
void loop() {
	if (radio.available()) {

        int text = -1;
		Serial.println(text, DEC);
        radio.read(&text, sizeof(text));
        Serial.println(text, DEC);
		delay(delayTime);
    }else{
		Serial.println("NOT AVAILABLE");
	}
	
    //Serial.print("TEST");
}

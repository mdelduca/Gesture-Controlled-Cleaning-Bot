#include <Arduino.h>

// code for Reciever to print hello world from Michael
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*
Reciever To Do:
Need to recieve:
  - Gyroscope Data
  - Flex Sensor Data

Needed Pin Connections:
  - GND (Pin 1)
  - Vcc (Pin 2)
  - 

Key operations:
with the gyroscope, we need to:
  - change steering? based on rotation
  - after a certain threshold rotation was made, we consider it a steering input

with the flex sensor, we need to:
  - change speed based on the "strength" of the input?



*/

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));
  delay(1000);
}
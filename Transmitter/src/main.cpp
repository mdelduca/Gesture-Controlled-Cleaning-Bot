
#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";
int count = 7;
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  const float text = count;
  radio.write(&text, sizeof(text));
  Serial.println(text);
  delay(1000);
  count++;
}
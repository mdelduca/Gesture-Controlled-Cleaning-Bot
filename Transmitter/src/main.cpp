
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
  const float text[3] = {count, count/2.0, pow(count, 0.5)};
  radio.write(&text, sizeof(text));
  Serial.print("Sent: ");
  Serial.print(text[0]);
  Serial.print(" ");
  Serial.print(text[1]);
  Serial.print(" ");
  Serial.println(text[2]);
  delay(1000);
  count++;
}
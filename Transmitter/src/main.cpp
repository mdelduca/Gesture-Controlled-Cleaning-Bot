
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
  // placeholders for the msg array
  const int msg[3] = {count, count/2, pow(count, 2)};
  radio.write(&msg, sizeof(msg));
  Serial.print("Sent: ");
  Serial.print(msg[0]);
  Serial.print(" ");
  Serial.print(msg[1]);
  Serial.print(" ");
  Serial.println(msg[2]);
  delay(1000);
  count++;
}

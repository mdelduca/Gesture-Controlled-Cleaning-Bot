#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

/*
roll/pitch effect steering
flex sensor effect acceleration
message format:
msg[0] flex value
msg[1] roll value
msg[2] pitch value
the above are hopefully calculated to a form that is useful
	- minimum thresholds have been handled

Flex Sensor:
physically returns values between 700-900
the glove will process it to become ___
this will be implemented in the calculations for the motor control output

Roll:
physically returns values between ___
the glove will process it to become ___
this will be implemented in the calculations for the motor control output

Pitch:
physically returns values between ___
the glove will process it to become ___
this will be implemented in the calculations for the motor control output

*** NEED TO FIGURE OUT CALCULATIONS AND VALUES TO IMPLEMENT IT TO THE MOTOR CONTROLS

motor control needs PWM Values (0-255)
sponge is fueled by the flex sensor (0-1020)

*/

int calculateSpongePWM(float);

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    float msg[3];
    radio.read(&msg, sizeof(msg));
	Serial.print("Recieved: ");
    Serial.print(msg[0], DEC);
	Serial.print(" ");
	Serial.print(msg[1], DEC);
	Serial.print(" ");
	Serial.println(msg[2], DEC);
	//int decoded = msg[0];

  }
}
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

#define LEFT_PIN A1
#define RIGHT_PIN A2
#define SPONGE_PIN A3

/*
roll/pitch effect steering
flex sensor effect acceleration
message format:
msg[0] pitch value
msg[1] roll value
msg[2] flex value

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
sponge is fueled by the flex sensor (0-1200)

need specific pins to control each motor
need the specific desired ratios on how each sensor should effect things

*/

int calculateSpongePWM(int gyro_pitch){
	// taking pitch in Degrees, converting to PWM
	// trying linear relationship
	return (gyro_pitch)*(255/90);
}



int roll_steer_threshold = 20;
int flex_threshold = 700;

int determineMoveState(int flex_value, int gyro_roll){
	if(abs(flex_value) >= flex_threshold){
		// we can move, figure out how we will moving
		if(gyro_roll >= roll_steer_threshold){
			// we are steering right
			return 3;
		}else if(gyro_roll <= -1*roll_steer_threshold){
			// we are steering left
			return 2;
		}else{
			// move forward
			return 1;
		}
	}else{
		return 0;
	}
	
}

int pitch_sponge_threshold = 20;

int determineSpongeState(int gyro_pitch){
	if(abs(gyro_pitch) >= pitch_sponge_threshold){
		// we are sponging now
		return 1;
	}else{
		// no sponge movement
		return 0;
	}
}

void setup() {
  	Serial.begin(9600);
	
	pinMode(LEFT_PIN, OUTPUT);
	pinMode(RIGHT_PIN, OUTPUT);
	pinMode(SPONGE_PIN, OUTPUT);
	
	radio.begin();
	radio.openReadingPipe(1, address);
	radio.setPALevel(RF24_PA_MIN);
	radio.startListening();
}

int flex_sensor = 0;
int roll = 0;
int pitch = 0;
int move_state = 0;
int sponge_state = 0;

int left_PWM = 0;
int right_PWM = 0;
int sponge_PWM = 0;

int accel_value = 50;
int low_steer_value = 25;
int high_steer_value = 75;
int sponge_value = 50;

void loop() {
  	if (radio.available()) {
		float msg[32];
		radio.read(msg, sizeof(msg));
		Serial.print("Recieved: ");
		Serial.print(msg[0], DEC);
		Serial.print(" ");
		Serial.print(msg[1], DEC);
		Serial.print(" ");
		Serial.println(msg[2], DEC);
		

		// manipulate motors based on the recieved sensor values
		flex_sensor = msg[0];
		roll = msg[1];
		pitch = msg[2];
		move_state = determineMoveState(flex_sensor, roll);
		sponge_state = determineSpongeState(pitch);

		if(move_state == 1){
			// move forward
			left_PWM = accel_value;
			right_PWM = accel_value;
		}else if(move_state == 2){
			// move left
			left_PWM = high_steer_value;
			right_PWM = low_steer_value;
		}else if(move_state == 3){
			// move right
			left_PWM = low_steer_value;
			right_PWM = high_steer_value;
		}else{
			// do nothing
			left_PWM = 0;
			right_PWM = 0;
		}

		if(sponge_state == 1){
			// sponging
			sponge_PWM = sponge_value;
		}else{
			// no sponging
			sponge_PWM = 0;
		}
		
  	}

	
	
	// output these PWM values to the corresponding pins
	analogWrite(LEFT_PIN, left_PWM);
	analogWrite(RIGHT_PIN, right_PWM);
	analogWrite(SPONGE_PIN, sponge_PWM);
	
}
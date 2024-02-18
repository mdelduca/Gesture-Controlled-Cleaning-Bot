#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 8
#define CFN_PIN 10

RF24 radio(CE_PIN, CFN_PIN); // CE, CSN

const byte address[6] = "00001";
/*
roll/pitch effect steering
flex sensor effect acceleration
Message Structure:
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

// ---------------- Wheel Motor Control ---------------
int motor1_fctrl = 2;               // direction/control motorA -fwd
int enA = 3;                        // motor speed
int motor1_bctrl = 4;               // direction/control motorA -bwd

int motor2_fctrl = 5;               // direction/control motorB -fwd
int enB = 6;                        // speed
int motor2_bctrl = 7;               // direction/control motorB -bwd

//----------------- Flex Sensor -----------------------
const int flexPin = A0;     // for flex sensor 
const int motorPin = 9;     // for motor
const int ledPin = 3;       // could add for backwards heading 

int smotorspeed;            // sponge motor
int lastValidReading = -1;  // Initialize with an invalid value to indicate that there's no previous reading
int force_resistance;


/*
0 = forward
1 = backward
2 = left
3 = right
4 = stop
stop if both angles are below threshold
forward/backward take priority, determined by pitch
*/
int determineMoveState(int gyro_roll, int gyro_pitch){
	
	if(gyro_pitch > 0){
		// move forwards
		return 0;
	}else if(gyro_pitch < 0 ){
		// move backwards
		return 1;
	}else if(gyro_roll > 0){
		// move left
		return 2;
	}else if(gyro_roll < 0){
		// move right
		return 3;
	}else{
		// stop
		return 4;
	}
}


void setup() {
  	Serial.begin(9600);
	
	radio.begin();
	radio.openReadingPipe(1, address);
	radio.setPALevel(RF24_PA_MIN);
	radio.startListening();

	// driving motor 
	pinMode(enA, OUTPUT);
	pinMode(motor1_fctrl, OUTPUT);
	pinMode(motor1_bctrl, OUTPUT);
	pinMode(enB, OUTPUT);
	pinMode(motor2_fctrl, OUTPUT);
	pinMode(motor2_bctrl, OUTPUT);

	// sponge motor
	pinMode(motorPin, OUTPUT);
}

int flex_sensor = 0;
int roll = 0;
int pitch = 0;
bool sponge_state = false;

/*
Message Structure:
3 floats
pitch, roll, then flex sensor
*/
int force_threshold = 75;
bool debounce = true;

void loop() {
  	if (radio.available()) {
		float msg[32];
		radio.read(msg, sizeof(msg));
		Serial.print("Recieved: ");
		Serial.print(msg[0], DEC);
		Serial.print(" ");
		Serial.print(msg[1], DEC);
		Serial.print(" ");
		//Serial.println(msg[2], DEC);
		

		// manipulate motors based on the recieved sensor values
		pitch = msg[0];
		roll = msg[1];
		flex_sensor = msg[2];

		// ============================  M O T O R   C O N T R O L  ===========================================
		// ====================================================================================================

		// orientation is received from the glove, gyroscopic sensor 
		int orientation = determineMoveState(roll, pitch);  // 0 = forwards, 1 = backwards, 2 = left, 3 = right, 4 = stop
		Serial.println(orientation, DEC);
		switch(orientation) {
			case 0:
				// move forwards
				digitalWrite(motor1_fctrl, LOW);
				digitalWrite(motor1_bctrl, HIGH);
				digitalWrite(motor2_fctrl, HIGH);
				digitalWrite(motor2_bctrl, LOW);
				analogWrite(enA, 150);//right wheel;
				analogWrite(enB, 100);//lreft wheel
			break;
			case 1:
				// move backwards
				digitalWrite(motor1_fctrl, HIGH);
				digitalWrite(motor1_bctrl, LOW);
				digitalWrite(motor2_fctrl, LOW);
				digitalWrite(motor2_bctrl, HIGH);
				analogWrite(enA, 150);//right wheel;
				analogWrite(enB, 100);//lreft wheel
			break;
			case 2:
				// move left
				digitalWrite(motor1_fctrl, LOW);
				digitalWrite(motor1_bctrl, HIGH);
				digitalWrite(motor2_fctrl, HIGH);
				digitalWrite(motor2_bctrl, LOW);
				analogWrite(enA, 100);//right wheel;
				analogWrite(enB, 50);//lreft wheel
			break;
			case 3:
				// move right
				digitalWrite(motor1_fctrl, LOW);
				digitalWrite(motor1_bctrl, HIGH);
				digitalWrite(motor2_fctrl, HIGH);
				digitalWrite(motor2_bctrl, LOW);
				analogWrite(enA, 100);//right wheel;
				analogWrite(enB, 100);//lreft wheel
			break;
			default:
				// stop
				digitalWrite(motor1_fctrl, LOW);
				digitalWrite(motor1_bctrl, LOW);
				digitalWrite(motor2_fctrl, LOW);
				digitalWrite(motor2_bctrl, LOW);
				digitalWrite(enA, 0);
				digitalWrite(enB, 0);
			break;
		}
		
		// ============================  F L E X  S E N S O R  ================================================
		// ====================================================================================================
		force_resistance = (int) flex_sensor;                       // reads voltage output from flex sensor 0-1023
		force_resistance = map(force_resistance, 0, 1023, 255, 0);    // maps the voltage output to a value between 0 and 255, max speed of motor
		Serial.println(force_resistance, DEC);

		
		if(force_resistance <= force_threshold && debounce){
			sponge_state = !sponge_state;
			debounce = false;
		}else if(force_resistance > force_threshold){
			debounce = true;
		}

		if(sponge_state){
			// move sponge
			smotorspeed = 150;
		}else{
			smotorspeed = 0;
		}

		//lastValidReading = force_resistance;
		/*
		if (lastValidReading == -1 || ( force_resistance >= lastValidReading * 0.9 || force_resistance <= lastValidReading * 1.1)) {
			// Update the last valid reading
			lastValidReading = force_resistance;
			// Process the current reading as it is valid
			//Serial.print("Valid reading: ");
			//Serial.println(force_resistance);
		} else {
			// Ignore the reading as it's an outlier
			//Serial.println("Outlier detected, ignoring reading.");
		} 
		*/
		//*** SET TO TOGGLE INSTEAD OF A GRADIENT
		// inverval adjustments for motor speed  
		/*                     
		if (lastValidReading > 0 && lastValidReading <= 50){
			smotorspeed = 50;
		} else if (lastValidReading > 50 && lastValidReading <= 100 ) {
			smotorspeed = 100;
		} else if (lastValidReading > 100 && lastValidReading <= 150 ){
			smotorspeed = 150;
		} else if (lastValidReading > 150 && lastValidReading <= 200 ){
			smotorspeed = 200;
		} else if (lastValidReading > 200){
			smotorspeed = 250;
		} else if (lastValidReading == 0){
			smotorspeed = 0;
		}*/

		//analogWrite(motorPin, smotorspeed); 
		analogWrite(motorPin, smotorspeed);                 // sets the speed of the motor 
		Serial.println(smotorspeed);

		//delay(100);		
  	}
}
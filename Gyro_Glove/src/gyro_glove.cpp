#include <Arduino.h>
#include <Wire.h>

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";




const int MPU = 0x68;  // MPU6050 I2C address
const int flexPin = A0;
const int ledPin = 4;
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float pitch, roll;
float gXRaw, gYRaw;
bool prevSet;
long unsigned int prevTime, currTime;
float elapsedTime;
float gyroXThresh, gyroYThresh;
float accXThresh, accYThresh;
float gRawXThresh, gRawYThresh;
int cycleCount;
float accAngleX, accAngleY;

float prevGyroX, prevGyroY;
bool dissmissRoll, dismissPitch;

void print();
void readGyro();
void readAcc();
void calcThreshold();
void calcGyroFix();
void sendData();

void setup() {
  Serial.begin(19200);
  pinMode(ledPin, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(ledPin, HIGH);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();


  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(20);
  prevSet = false;
  calcThreshold();
  calcGyroFix();
  delay(20);
  prevSet = false;
  cycleCount = 0;
  digitalWrite(ledPin, LOW);
}

void loop() {
  cycleCount++;
  if (!prevSet) {
    prevTime = millis();
    prevSet = true;
  }
  //  Accelerometer Data Retrieval
  readAcc();

  currTime = millis();
  elapsedTime = (currTime - prevTime) / 1000.0;
  prevTime = currTime;

  //  Gyro Data Retrieval
  readGyro();

  // gyroX = map(gyroX, -255, 255, -155, 155);
  // gyroY = map(gyroY, -255, 255, -155, 155);

  if (cycleCount == 25) {
    cycleCount = 0;
    calcGyroFix();
    gXRaw -= (gRawXThresh);
    gYRaw -= (gRawYThresh);
  }

  if (abs(gyroX) < abs(gyroXThresh)) {
    gyroX = 0.0;
  }

  if (abs(gyroY) < abs(gyroYThresh)) {
    gyroY = 0.0;
  }
  accAngleX = (accAngleX - (accXThresh * (accAngleX / abs(accAngleX))));
  accAngleY = (accAngleY - (accYThresh * (accAngleY / abs(accAngleY))));

  if (gyroX != 0) {
    gXRaw += (gyroX - (gyroXThresh * (gyroX / abs(gyroX)))) * elapsedTime;
  }

  if (gyroY != 0) {
    gYRaw += (gyroY - (gyroYThresh * (gyroY / abs(gyroY)))) * elapsedTime;
  }

  // if (abs(gyroX - prevGyroX) > 100 && pitch == 0) {
  //   gXRaw = 0;
  //   gyroX = 0;
  // }

  // if (abs(gyroY - prevGyroY) > 120 && roll == 0) {
  //   gYRaw = 0;
  //   gyroY = 0;
  // }

  pitch = 0.96 * gXRaw + 0.04 * accAngleX;
  roll = 0.96 * gYRaw + 0.04 * accAngleY + 5;

  if (abs(pitch) < 40) {
    pitch = 0;
    // gXRaw = 0;
    accAngleX = 0;
  }

  if (abs(roll) < 40) {
    roll = 0;
    // gYRaw = 0;
    accAngleY = 0;
  }

  if (pitch == 0 && abs(gyroX) == 0) {
    gXRaw = 0;
  }

  if (roll == 0 && abs(gyroY) == 0) {
    gYRaw = 0;
  }

  print();

  prevGyroX = gyroX;
  prevGyroY = gyroY;
  // pitch > 0, set d2 to high
  // roll < 0, set d4 to high
  // pitch < 0, set d3 to high
  // roll > 0, set d5 to high

  if(pitch > 0){
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
  }else if(pitch < 0){
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
  }

  if(roll > 0){
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
  }else if(roll < 0){
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
  sendData();
}

void readAcc() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  // ACCX
  accX = (Wire.read() << 8 | Wire.read()) / 16384.0;

  // ACCY
  accY = (Wire.read() << 8 | Wire.read()) / 16384.0;

  // ACCZ
  accZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  accAngleX = (atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / PI) -
              0.58;  // AccErrorX ~(0.58) See the calculate_IMU_error()custom
                     // function for more details
  accAngleY =
      (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / PI) + 1.58;
}

void readGyro() {
  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  // GYROX
  gyroX = (Wire.read() << 8 | Wire.read()) / 131.0;

  // GYROY
  gyroY = (Wire.read() << 8 | Wire.read()) / 131.0;

  // GYROZ
  gyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
}

void calcThreshold() {
  float sumAX = 0.0;
  float sumAY = 0.0;
  float sumGX = 0.0;
  float sumGY = 0.0;
  for (int i = 0; i < 15; i++) {
    readAcc();

    sumAX += accAngleX;
    sumAY += accAngleY;

    accXThresh =
        abs(accXThresh) < abs(accAngleX) ? abs(accAngleX) : abs(accXThresh);

    accYThresh =
        abs(accYThresh) < abs(accAngleY) ? abs(accAngleY) : abs(accYThresh);
  }
  for (int i = 0; i < 250; i++) {
    readGyro();

    sumGX += gyroX;
    sumGY += gyroX;

    gyroXThresh = abs(gyroXThresh) < abs(gyroX) ? abs(gyroX) : abs(gyroXThresh);
    gyroYThresh = abs(gyroYThresh) < abs(gyroY) ? abs(gyroY) : abs(gyroYThresh);
    Serial.print(gyroXThresh);
    Serial.print(" / ");
    Serial.println(gyroYThresh);
  }

  Serial.print("GyroX: ");
  Serial.print(gyroXThresh);
  Serial.print("            ");

  Serial.print("GyroY: ");
  Serial.println(gyroYThresh);

  Serial.print("AccX: ");
  Serial.print(accXThresh);
  Serial.print("            ");

  Serial.print("AccY: ");
  Serial.println(accYThresh);
}

void calcGyroFix() {
  float sumGX = 0.0;
  float sumGY = 0.0;
  float diffX;
  float diffY;
  for (int i = 0; i < 10; i++) {
    if (!prevSet) {
      prevTime = millis();
      prevSet = true;
    }
    currTime = millis();
    elapsedTime = (currTime - prevTime) / 1000.0;
    prevTime = currTime;
    readGyro();

    // gyroX = map(gyroX, -255, 255, -155, 155);
    // gyroY = map(gyroY, -255, 255, -155, 155);

    if (abs(gyroX) < abs(gyroXThresh)) {
      gyroX = 0.0;
    }

    if (abs(gyroY) < abs(gyroYThresh)) {
      gyroY = 0.0;
    }

    if (gyroX != 0) {
      gXRaw += (gyroX - (gyroXThresh * (gyroX / abs(gyroX)))) * elapsedTime;
    }

    if (gyroY != 0) {
      gYRaw += (gyroY - (gyroYThresh * (gyroY / abs(gyroY)))) * elapsedTime;
    }

    sumGX += gXRaw;
    sumGY += gYRaw;

    if (i == 0) {
      diffX += gXRaw;
      diffY += gYRaw;
    }

    if (i == 8) {
      diffX -= gXRaw;
      diffY -= gYRaw;
    }

    // Serial.print(diffX);
    // Serial.print(" / ");
    // Serial.println(sumGX / 10.0);
  }
  Serial.println(sumGX / 10.0);

  if (pitch == 0 && gyroX == 0) {
    gRawXThresh = gXRaw;

  } else if (abs(diffX) < 1) {
    gRawXThresh = diffX;
  } else {
    gRawXThresh = 0;
  }

  if (roll == 0 && gyroY == 0) {
    gRawYThresh = gYRaw;
  } else if (abs(diffY) < 0.3) {
    gRawYThresh = diffY;
  } else {
    gRawYThresh = 0;
  }

  // Serial.print("GyroX: ");
  // Serial.print(gRawXThresh);
  // Serial.print("            ");

  // Serial.print("GyroY: ");
  // Serial.println(gRawYThresh);
}

void sendData() {
  const float text[3] = {pitch, roll, analogRead(flexPin)};
  radio.write(&text, sizeof(text));
  Serial.print("Sent: ");
  Serial.print(text[0]);
  Serial.print(" ");
  Serial.print(text[1]);
  Serial.print(" ");
  Serial.println(text[2]);
}

void print() {
  // Serial.print("AccX: ");
  // Serial.print(accX);
  // Serial.print("            ");

  // Serial.print("AccY: ");
  // Serial.println(accY);

  // Serial.print("GyroY: ");
  // Serial.print(gyroY);
  // Serial.print("            ");

  // Serial.print("GyroYRaw: ");
  // Serial.print(gYRaw);
  // Serial.print("            ");

  // Serial.print("GyroZ: ");
  // Serial.print(gyroZ);
  // Serial.print("            ");

  // Serial.print("Roll: ");
  // Serial.print(roll);
  // Serial.print("            ");

  // Serial.print("Pitch: ");
  // Serial.println(pitch);
}

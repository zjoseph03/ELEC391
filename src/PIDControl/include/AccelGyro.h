#include <Arduino.h>
#include "Arduino_BMI270_BMM150.h"

float k = 0.95;  // Complementary filter coefficient
float lastRollFiltered = 0;
float lastPitchFiltered = 0;
unsigned long previousTime = 0;
unsigned long currentTime = 0;
volatile bool sampleFlag = false;
int missedSamples = 0;
float dt;

// Angle Struct for storing all angle data for filtered angles, gyro angles, and accelerometer angles
typedef struct gyroData {
  float gx;
  float gy;
  float gz;
} gyroData_S;
gyroData_S gyroData;

typedef struct accelData {
  float ax;
  float ay;
  float az;
} accelData_S;
accelData_S accelData;

typedef struct angleData {
  float pitchRate;
  float rollRate;
  float accelRoll;
  float accelPitch;
  float gyroRoll = 0;
  float gyroPitch = 0;
  float rollFiltered;
  float pitchFiltered;
} angleData_S; 
angleData_S angleData;

void getAccelData() {
  float ax, ay, az;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    accelData.ax = ax / 8192.0;
    accelData.ay = ay / 8192.0;
    accelData.az = az / 8192.0;
  } 
}

// FIX: Find precise scaling factor for the gyroscope data
void getGyroData() {
  float gx, gy, gz;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    gyroData.gx = (gx / 17.2);
    gyroData.gy = (gy / 17.2);
    gyroData.gz = (gz / 17.2);
  }
}

void calculateAngles() {
  currentTime = millis();
  dt = (currentTime - previousTime) / 1000.0; // in seconds
  previousTime = currentTime;

  // Accelerometer-based angles (in degrees) - KEEP THIS PART
  angleData.accelRoll = atan2(-accelData.ax, sqrt(accelData.ay * accelData.ay + accelData.az * accelData.az)) * 180.0 / PI;
  angleData.accelPitch = atan2(accelData.ay, sqrt(accelData.ax * accelData.ax + accelData.az * accelData.az)) * 180.0 / PI;

  angleData.rollRate = gyroData.gy;  // X-axis for roll rate
  angleData.pitchRate = gyroData.gz; // Y-axis for pitch rate
  
  angleData.gyroRoll =  (angleData.rollRate * dt);
  angleData.gyroPitch = (angleData.pitchRate * dt);

  // Serial.print("Dt: ");
  // Serial.println(dt);
  // Serial.print("Gyro Gy: ");
  // Serial.println(gyroData.gy);
  // Serial.print("Gyro Roll Angle: ");
  // Serial.println(angleData.gyroRoll);
}

void calculateFilteredAngles() {
  // Gyroscope drift correction at small angles
  // if (angleData.accelRoll < 0.15 && angleData.accelRoll > -0.15) {
  //   angleData.gyroRoll = angleData.accelRoll;
  //   lastRollFiltered = angleData.accelRoll;
  // }
  
  angleData.rollFiltered = (k * (lastRollFiltered + angleData.gyroRoll)) + 
                        ((1-k) * angleData.accelRoll);
  angleData.pitchFiltered = (k * (lastPitchFiltered + angleData.gyroPitch)) + 
                          ((1-k) * angleData.accelPitch);

  lastRollFiltered = angleData.rollFiltered;
  lastPitchFiltered = angleData.pitchFiltered;

  // Serial.print("Filtered Roll: ");
  // Serial.println(angleData.rollFiltered);
}

// We can split this into two different flags for the accelleromotor and gyroscope if we need to sample at different rates
void sampleSensors() {
  sampleFlag = true;
  missedSamples++; // We can use this if we run into problems with missing samples at the current freq. 
}

void printData() {
  Serial.println();
  Serial.println("Gyro Data: ");
  Serial.println(gyroData.gx);
  Serial.println(gyroData.gy);
  Serial.println(gyroData.gz);


  Serial.println();
  Serial.println("Accel Data: ");
  Serial.println(accelData.ax);
  Serial.println(accelData.ay);
  Serial.println(accelData.az);

  Serial.print("\nRoll: ");
  Serial.println(angleData.rollFiltered,6);
  Serial.println();
}
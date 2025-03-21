#include <Arduino.h>
#include "Arduino_BMI270_BMM150.h"

float k = 0.98;  // Complementary filter coefficient
float lastRollFiltered = 0;
float lastPitchFiltered = 0;
unsigned long previousTime = 0;
volatile bool sampleFlag = false;
int missedSamples = 0;

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
  float gyroRoll;
  float gyroPitch;
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

void getGyroData() {
  float gx, gy, gz;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    gyroData.gx = gx;
    gyroData.gy = gy;
    gyroData.gz = gz;
  }
}

void calculateAngles() {
  // Time step calculation - FIX: Calculate proper delta time
  unsigned long currentTime = millis();
  float dt = (currentTime - previousTime) / 1000.0; // in seconds
  previousTime = currentTime;

  // Accelerometer-based angles (in degrees) - KEEP THIS PART
  angleData.accelRoll = atan2(-accelData.ax, sqrt(accelData.ay * accelData.ay + accelData.az * accelData.az)) * 180.0 / PI;
  angleData.accelPitch = atan2(accelData.ay, sqrt(accelData.ax * accelData.ax + accelData.az * accelData.az)) * 180.0 / PI;

  // FIX: Correct gyro rate calculation with proper axis mapping
  angleData.rollRate = gyroData.gx * dt;  // X-axis for roll rate
  angleData.pitchRate = gyroData.gy * dt; // Y-axis for pitch rate
  
  // FIX: Proper gyro angle integration
  angleData.gyroRoll += angleData.rollRate;
  angleData.gyroPitch += angleData.pitchRate;
}

void calculateFilteredAngles() {
  // Complementary filter
  angleData.rollFiltered = (k * (lastRollFiltered + angleData.rollRate)) + 
                        ((1-k) * angleData.accelRoll);
  angleData.pitchFiltered = (k * (lastPitchFiltered + angleData.pitchRate)) + 
                          ((1-k) * angleData.accelPitch);

  lastRollFiltered = angleData.rollFiltered;
  lastPitchFiltered = angleData.pitchFiltered;
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
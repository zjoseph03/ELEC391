#include <Arduino.h>
#include "Arduino_BMI270_BMM150.h"

float k = 0.01;  // Complementary filter coefficient
float lastRollFiltered = 0;
float lastPitchFiltered = 0;
unsigned long previousTime = 0;

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

void getAccelData(float* accelDataTemp) {
  float ax, ay, az;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    accelData.ax = ax / 8192.0;
    accelData.ay = ay / 8192.0;
    accelData.az = az / 8192.0;

    // accelData.ax = ax_g;
    // accelData.ay = ay_g;
    // accelData.az = az_g;
  }
}

void getGyroData(float* gyroDataTemp) {
  float gx, gy, gz;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    gyroData.gx = gx;
    gyroData.gy = gy;
    gyroData.gz = gz;
  }
}

void calculateAngles(float* accelDataTemp, 
                     float* gyroDataTemp) {
  // Time step calculation
  unsigned long currentTime = millis();
  previousTime = currentTime;

  // Accelerometer-based angles (in degrees)
  // atan(-x / sqrt(y^2 + z^2)) for roll
  // atan( y / sqrt(x^2 + z^2)) for pitch
  angleData.accelRoll = atan2(-accelData.ax, sqrt(accelData.ay * accelData.ay + accelData.az * accelData.az)) * 180.0 / PI;
  angleData.accelPitch = atan2(accelData.ay, sqrt(accelData.ax * accelData.ax + accelData.az * accelData.az)) * 180.0 / PI;

  // Gyroscope-based angles (in degrees)
  float gyroDeltaT = 1 / IMU.gyroscopeSampleRate();

  angleData.gyroRoll = angleData.accelRoll + (gyroData.gz * gyroDeltaT);
  angleData.gyroPitch = angleData.accelPitch + (gyroData.gz * gyroDeltaT);
  
  // Gyroscope angle integration (rate of change to angle)
  angleData.pitchRate = gyroData.gz * gyroDeltaT;
  angleData.rollRate = gyroData.gy * gyroDeltaT;
}

void calculateFilteredAngles(float* gyroDataTemp,
                             float* accelDataTemp) {
  // Complementary filter
  angleData.rollFiltered = (k * (lastRollFiltered + angleData.rollRate)) + 
                        ((1-k) * angleData.accelRoll);
  angleData.pitchFiltered = (k * (lastPitchFiltered + angleData.pitchRate)) + 
                          ((1-k) * angleData.accelPitch);

  lastRollFiltered = angleData.rollFiltered;
  lastPitchFiltered = angleData.pitchFiltered;
}
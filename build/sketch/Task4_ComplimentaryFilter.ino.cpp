#include <Arduino.h>
#line 1 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
#include "Arduino_BMI270_BMM150.h"
#include <cmath>

// Global Variables
float k = 0.01;  // Complementary filter coefficient
float lastRollFiltered = 0;
float lastPitchFiltered = 0;
unsigned long previousTime = 0;

// Angle Struct for storing all angle data for filtered angles, gyro angles, and accelerometer angles
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

#line 23 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void setup();
#line 32 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void getAccelData(float* accelData);
#line 47 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void getGyroData(float* gyroData);
#line 57 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void calculateAngles(float* accelData, float* gyroData);
#line 80 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void calculateFilteredAngles(float* gyroData, float* accelData);
#line 92 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void loop();
#line 23 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
void setup() {
  Serial.begin(9600);
  while (!Serial);
  if (!IMU.begin()) {
    while (1);  // Stop if IMU initialization fails
  }
  previousTime = millis();
}

void getAccelData(float* accelData) {
  float ax, ay, az;
  float ax_g, ay_g, az_g;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    ax_g = ax / 8192.0;
    ay_g = ay / 8192.0;
    az_g = az / 8192.0;

    accelData[0] = ax_g;
    accelData[1] = ay_g;
    accelData[2] = az_g;
  }
}

void getGyroData(float* gyroData) {
  float gx, gy, gz;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    gyroData[0] = gx;
    gyroData[1] = gy;
    gyroData[2] = gz;
  }
}

void calculateAngles(float* accelData, 
                     float* gyroData) {
  // Time step calculation
  unsigned long currentTime = millis();
  previousTime = currentTime;

  // Accelerometer-based angles (in degrees)
  // atan(-x / sqrt(y^2 + z^2)) for roll
  // atan( y / sqrt(x^2 + z^2)) for pitch
  angleData.accelRoll = atan2(-accelData[0], sqrt(accelData[1] * accelData[1] + accelData[2] * accelData[2])) * 180.0 / PI;
  angleData.accelPitch = atan2(accelData[1], sqrt(accelData[0] * accelData[0] + accelData[2] * accelData[2])) * 180.0 / PI;

  // Gyroscope-based angles (in degrees)
  float gyroDeltaT = 1 / IMU.gyroscopeSampleRate();

  angleData.gyroRoll = angleData.accelRoll + (gyroData[2] * gyroDeltaT);
  angleData.gyroPitch = angleData.accelPitch + (gyroData[2] * gyroDeltaT);
  
  // Gyroscope angle integration (rate of change to angle)
  angleData.pitchRate = gyroData[2] * gyroDeltaT;
  angleData.rollRate = gyroData[1] * gyroDeltaT;
}

void calculateFilteredAngles(float* gyroData,
                             float* accelData) {
  // Complementary filter
  angleData.rollFiltered = (k * (lastRollFiltered + angleData.rollRate)) + 
                        ((1-k) * angleData.accelRoll);
  angleData.pitchFiltered = (k * (lastPitchFiltered + angleData.pitchRate)) + 
                          ((1-k) * angleData.accelPitch);

  lastRollFiltered = angleData.rollFiltered;
  lastPitchFiltered = angleData.pitchFiltered;
}

void loop() {
  float accelData[3];
  float gyroData[3];

  // Get acceleration data
  getAccelData(accelData);

  // Gyroscope Readings and Complementary Filter
  getGyroData(gyroData);

  // Calculate pitch and roll for gyroscope and accelerometer
  calculateAngles(accelData, gyroData);
  
  // Apply filter to roll and pitch angles
  calculateFilteredAngles(gyroData, accelData);

  // Output all values in a single comma-separated line:
  // Format: ax,ay,az,rollAcc,pitchAcc,gx,gy,gz,rollGyro,pitchGyro,rollFiltered,pitchFiltered
  Serial.print(accelData[0], 6); Serial.print(",");
  Serial.print(accelData[1], 6); Serial.print(",");
  Serial.print(accelData[2], 6); Serial.print(",");

  Serial.print(angleData.accelRoll, 6); Serial.print(",");
  Serial.print(angleData.accelPitch, 6); Serial.print(",");

  Serial.print(gyroData[0], 6); Serial.print(",");
  Serial.print(gyroData[1], 6); Serial.print(",");
  Serial.print(gyroData[2], 6); Serial.print(",");

  Serial.print(angleData.rollRate, 6); Serial.print(",");
  Serial.print(angleData.pitchRate, 6); Serial.print(",");

  Serial.print(angleData.rollFiltered, 6); Serial.print(",");
  Serial.print(angleData.pitchFiltered, 6); Serial.print(",");
  Serial.println(angleData.gyroRoll, 6); Serial.print("\n");
}


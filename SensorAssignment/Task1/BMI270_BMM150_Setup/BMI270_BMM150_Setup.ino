/*
  Arduino LSM9DS1 - Simple Accelerometer
  This example reads the acceleration values from the LSM9DS1
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.
  The circuit:
  - Arduino Nano 33 BLE Sense
  created 10 Jul 2019
  by Riccardo Rizzo
  This example code is in the public domain.
*/
#include "Arduino_BMI270_BMM150.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);
//  Serial.println("Started");
  if (!IMU.begin()) {
//    Serial.println("Failed to initialize IMU!");
    while (1);
  }
//  Serial.print("Accelerometer sample rate = ");
//  Serial.print(IMU.accelerationSampleRate());
//  Serial.println(" Hz");
//  Serial.println();
//  Serial.println("Acceleration in G's");
//  Serial.println("X\tY\tZ");
}
void loop() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    // Convert raw values to g-force (assuming +/-2g range)
    float ax_g = x / 8192;
    float ay_g = y / 8192;
    float az_g = z / 8192;
    // Calculate angles in degrees
    float roll = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
    float pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;
    // In the loop() function, replace the serial printing with:
    Serial.print(ax_g);
    Serial.print(',');
    Serial.print(ay_g);
    Serial.print(',');
    Serial.print(az_g);
    Serial.print(',');
    Serial.print(roll);
    Serial.print(',');
    Serial.println(pitch);  // Only one println at the end

  }
}

/*
  Arduino LS\
  ]=M9DS1 - Simple Accelerometer

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
#include <cmath>

void setup() {
  Serial.begin(9600);
  while (!Serial);
//  Serial.println("Started");

  if (!IMU.begin()) {
//    Serial.println("Failed to initialize IMU!");
    while (1);
  }

}

void loop() {
  float x, y, z;
  float theta = 0;
  float roll;
  float pitch;
  float yaw;
  float ax_g, ay_g, az_g;


  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    // Convert raw values to g-force (assuming +/-2g range)
    ax_g = x / 8192;
    ay_g = y / 8192;
    az_g = z / 8192;

    // Calculate angles in degrees
    roll = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;
    pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;

    // In the loop() function, replace the serial printing with:
//    Serial.println("Acceleramator X Y Z Roll Pitch");
//    Serial.println(String(ax_g) + ',' + String(ay_g) + ',' + String(az_g) + ',' + String(roll) + ',' + String(pitch));

    
    
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
    

//    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    float deltaT = 1 / IMU.gyroscopeSampleRate();
    float roll0 = roll;
    float pitch0 = pitch;

    float thetaRoll = roll0 + (z * deltaT);
    float thetaPitch = pitch0 + (z * deltaT);
    
//    Serial.println(" Hz");
//    Serial.println();
//    Serial.println("Angular speed in degrees/second");
//    Serial.println(String(x) + ',' + String(y) + ',' + String(z)); 
    Serial.println("Roll, Pitch"); 
    Serial.println(String(thetaRoll) + ',' + String(thetaPitch));

    // Serial.println("aX aY aZ aRoll aPitch gX gY gZ gRoll gPitch");
    Serial.println(String(ax_g) + ',' + String(ay_g) + ',' + String(az_g) + ',' + String(roll) + ',' + String(pitch) + ',' + String(x) + ',' + String(y) + ',' + String(z) + ',' + String(thetaRoll) + ',' + String(thetaPitch));

    }
}

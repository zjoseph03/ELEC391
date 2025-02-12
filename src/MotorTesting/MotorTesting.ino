#include "Arduino_BMI270_BMM150.h"
#include "include/ELEC391MotorDriver.h" // NOTE: FIX THIS SO IT"S NOT DEPENDENT ON MY LOCAL PATH
#include <cmath>

/*
  Purpose of this file is purely for example code on using the motors
*/

// Global Variables

// Angle Struct for storing all angle data for filtered angles, gyro angles, and accelerometer angles
// NOTE: May need to also power nSLEEP for motor driver to work and may need to read back nFAULT 

MotorDriver rightMotor(5, 4);
MotorDriver leftMotor(3, 2);

void setup() {
  Serial.begin(9600);
  
}

void loop() {
  // Example code where we can go forward at 25%, 50%, 75%, 100% speed with 2 seconds between each
  // Then go backwards at 25%, 50%, 75%, 100% speed with 2 seconds between each
  // We need to define one PWMController object for the forwards and backwards PWM
  // For the motor control library, we can init an object with the input being two pins
  // Then we can have methods in the motor obkect to go forward or backward with the input being the speed

  leftMotor.forward(100);
  // rightMotor.forward(100);
  // delay(1000);
  // rightMotor.backward(100);
  // delay(1000);
  // rightMotor.stop();
  // delay(1000);
  // leftMotor.forward(100);
  // delay(1000);
  // leftMotor.backward(100);
  // delay(1000);
  // leftMotor.stop();
}

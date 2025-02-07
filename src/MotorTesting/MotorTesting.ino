#include "Arduino_BMI270_BMM150.h"
#include "include/ELEC391MotorDriver.h" // NOTE: FIX THIS SO IT"S NOT DEPENDENT ON MY LOCAL PATH
#include <cmath>

/*
  Purpose of this file is purely for example code on using the motors
*/

// Global Variables

// Angle Struct for storing all angle data for filtered angles, gyro angles, and accelerometer angles
// NOTE: May need to also power nSLEEP for motor driver to work and may need to read back nFAULT 

PWMController pwmD9;
PWMController pwmD10;

void setup() {
  Serial.begin(9600);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pwmD9.init(D9);
  pwmD10.init(D10);
}

void loop() {
  // pwmD10.setFrequency(500);
  // pwmD9.setFrequency(500);
  // pwmD10.writePWMDutyCycle(0);
  // pwmD9.writePWMDutyCycle(100);

  // Example code where we can go forward at 25%, 50%, 75%, 100% speed with 2 seconds between each
  // Then go backwards at 25%, 50%, 75%, 100% speed with 2 seconds between each
  // We need to define one PWMController object for the forwards and backwards PWM
  // For the motor control library, we can init an object with the input being two pins
  // Then we can have methods in the motor obkect to go forward or backward with the input being the speed
  // Eg) 
  // motor.init(FORWARDS_PIN, BACKWARDS_PIN)
  // motor.forward(25)
  // motor.backward(25)
  // Within the motor class, 
  delay(10);
}

#include "Arduino_BMI270_BMM150.h"
#include "D:\Courses\ELEC391\include\ELEC391PWM.h" // NOTE: FIX THIS SO IT"S NOT DEPENDENT ON MY LOCAL PATH
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
  // pwmD10.setFrequency(1000);
  // pwmD9.setFrequency(500);
  // pwmD9.writePWMDutyCycle(D9, 25);
  // pwmD10.writePWMDutyCycle(D10, 50);

}

void loop() {
  pwmD10.setFrequency(500);
  pwmD9.setFrequency(500);
  pwmD10.writePWMDutyCycle(D10, 0);
  pwmD9.writePWMDutyCycle(D9, 100);
  delay(10 );
}

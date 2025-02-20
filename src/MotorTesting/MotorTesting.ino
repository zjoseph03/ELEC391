#include "Arduino_BMI270_BMM150.h"
#include "include/ELEC391MotorDriver.h" 

// Global pointers for heap allocation
MotorDriver* rightMotor;
MotorDriver* leftMotor;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial to be ready

  // Allocate objects on heap
  rightMotor = new MotorDriver(5, 4);
  leftMotor = new MotorDriver(3, 2);

  Serial.println("MotorDriver instances created");
}

void loop() {
  Serial.println("Left forward at 50. Right backwards at 25");
  leftMotor->forward(50);
  rightMotor->forward(25);
  delay(1000);

  Serial.println("Left backward at 25. Right backward at 50");
  rightMotor->backward(50);
  leftMotor->backward(25);
  delay(1000);

  Serial.println("Both motors stopped");
  rightMotor->stop();
  leftMotor->stop();
  delay(1000);

  Serial.println("Left backward at 75. Right backward at 25");
  rightMotor->backward(25);
  leftMotor->backward(75);
  delay(1000);

  Serial.println("Both motors stopped");
  rightMotor->stop();
  leftMotor->stop();
  delay(1000);
}

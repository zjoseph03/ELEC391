#include <Arduino.h>
#include "include/AccelGyro.h"
#include "include/ELEC391PWM.h"



PWMController pwmController(2); // motor 1 forward
PWMController pwmController2(3); // motor 1 backward
PWMController pwmController3(4); // motor 2 forward
PWMController pwmController4(5); // motor 2 backward

void setup() {
  Serial.begin(9600);
  while (!Serial);
  if (!IMU.begin()) {
    while (1);  // Stop if IMU initialization fails
  }
  
  previousTime = millis();
  // pinMode(2, OUTPUT);
  // pinMode(3, OUTPUT);
  // pinMode(4, OUTPUT);
  // pinMode(5, OUTPUT);

  // NOTE: WRITE DEFAULT FOR THIS!!!!
  pwmController.setFrequency(500);  // Change to 500Hz
  pwmController2.setFrequency(500);
  pwmController3.setFrequency(500);  // Change to 500Hz
  pwmController4.setFrequency(500);
}

void loop() {
  float accelDataTemp[3];
  float gyroDataTemp[3];
  float roll;
  float dutyCycle;
  float dutyCycleBackwards;
  int i;

  getGyroData(gyroDataTemp);
  getAccelData(accelDataTemp);

  calculateAngles(accelDataTemp, gyroDataTemp);
  calculateFilteredAngles(gyroDataTemp, accelDataTemp);

  Serial.print("\n\n\n");

    Serial.print("Gyro Data ");
    Serial.print(": ");
    Serial.println(gyroData.gx);
    Serial.println(gyroData.gy);
    Serial.println(gyroData.gz);


  Serial.print("\n");
    Serial.print("Accel Data ");
    Serial.print(": ");
    Serial.println(accelData.ax);
    Serial.println(accelData.ay);
    Serial.println(accelData.az);

  Serial.print("Roll: ");
  Serial.println(angleData.rollFiltered,6);
  Serial.print("\n");
  if (angleData.rollFiltered > 2) {
    Serial.println("FORWARD MOVEMENT");
    roll = constrain(angleData.rollFiltered, 1, 15);
    // Roll = 30 degrees means move motors forward 100%
    dutyCycle = (roll * (100.0 / 15.0));
    dutyCycleBackwards = 0.0;
    // analogWrite(2, 0);
    // analogWrite(3, dutyCycle);
    // analogWrite(4, 0);
    // analogWrite(5, dutyCycle);
    
    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(dutyCycle); 
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(dutyCycle);
    Serial.print("Motor 1 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 1 Backward: ");
    Serial.println(dutyCycleBackwards, 6);
    Serial.print("Motor 2 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 2 Backward: ");
    Serial.println(dutyCycleBackwards, 6);

  } else if (angleData.rollFiltered < -2 ) {
    Serial.println("BACKWARDS MOVEMENT");
    roll = constrain(angleData.rollFiltered, -15, -1);
    dutyCycleBackwards = -1 * (roll * (100.0 / 15.0));
    dutyCycle = 0.0;

    // analogWrite(2, dutyCycleBackwards);
    // analogWrite(3, 0);
    // analogWrite(4, dutyCycleBackwards);
    // analogWrite(5, 0);

    pwmController.writePWMDutyCycle(dutyCycleBackwards);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(dutyCycleBackwards);
    pwmController4.writePWMDutyCycle(0);


    Serial.print("Motor 1 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 1 Backward: ");
    Serial.println(dutyCycleBackwards, 6);
    Serial.print("Motor 2 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 2 Backward: ");
    Serial.println(dutyCycleBackwards, 6);
  } else {
    dutyCycleBackwards = 0.0;
    dutyCycle = 0.0;

    Serial.println("NO MOVEMENT");
    // analogWrite(2, 0);
    // analogWrite(3, 0);
    // analogWrite(4, 0);
    // analogWrite(5, 0);

    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(0);

    Serial.print("Motor 1 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 1 Backward: ");
    Serial.println(dutyCycleBackwards, 6);
    Serial.print("Motor 2 Forward: ");
    Serial.println(dutyCycle, 6);
    Serial.print("Motor 2 Backward: ");
    Serial.println(dutyCycleBackwards, 6);
  }

}
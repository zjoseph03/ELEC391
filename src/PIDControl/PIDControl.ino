
#include <Arduino.h>
#include "include/AccelGyro.h"
#include "include/ELEC391PWM.h"
PWMController pwmController(2); // motor 1 forward
PWMController pwmController2(3); // motor 1 backward
PWMController pwmController3(4); // motor 2 forward
PWMController pwmController4(5); // motor 2 backward
// PID tuning constants
double kpValues[] = {0.001, 0.0015, 0.002, 0.0025};
int currentKpIndex = 0; 
unsigned long kpTestStartTime;
const unsigned long kpTestDuration = 20000;
double Kp = 8.0;   // Proportional gain
float Ki = 1;    // Integral gain
float Kd = 0.1;    // Derivative gain
// PID variables
double pidError = 0, previousError = 0;
float integral = 0, derivative = 0;
double output = 0;
float setpoint = 0;  // Target angle (upright position)

mbed::Ticker samplingTicker;
const int samplingFreq = 100; // Sample sensor at 100 Hz (every 10ms). Need to experiment to see what sampling freq we can use

// pidPreviousTime is assumed to be defined in one of your libraries; if not, declare it here:
unsigned long pidPreviousTime;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!IMU.begin()) {
    while (1);  // Stop if IMU initialization fails
  }
  
  pidPreviousTime = millis();
  Kd = kpValues[currentKpIndex];
  kpTestStartTime = millis();
  
  Serial.print("Starting test with Kp = ");
  Serial.println(Kp);
  
  // Set PWM frequency to 500Hz for all motor controllers
  pwmController.setFrequency(2000);
  pwmController2.setFrequency(2000);
  pwmController3.setFrequency(2000);
  pwmController4.setFrequency(2000);
  samplingTicker.attach(mbed::callback(sampleSensors), std::chrono::milliseconds(1000 / samplingFreq));
}

void loop() {
  unsigned long currentTime = millis();
  // Arrays to store temporary sensor data
  float accelDataTemp[3];
  float gyroDataTemp[3];
  float dutyCycle;
  float dutyCycleBackwards;
  
  // if (sampleFlag) {
    // Serial.print("Missed samples: ");
    // Serial.println(missedSamples);
    currentTime = millis();
    Kp = 2.0;
    Ki = 0;
  
    // Check if it's time to switch to the next Kp value
    if (currentTime - kpTestStartTime >= kpTestDuration) {
      // Move to next Kp value
      currentKpIndex++;

      
      // Reset integral to prevent accumulated error affecting next test
      integral = 0;


      
      // Check if we've tested all values
      if (currentKpIndex < 4) {
        Kd = kpValues[currentKpIndex];
        kpTestStartTime = currentTime;
        
        Serial.print("Switching to Kd = ");
        Serial.println(Kd, 6);
      } else {
        // All tests complete
        Serial.println("All Kp tests complete");
        // Stop motors
        stopAllMotors();
        while(1); // Halt program or implement a restart mechanism
      }
    }
      
      // Kp = 8.0;   // Proportional gain
      if (sampleFlag) {
        getAccelData();
        getGyroData();

        calculateAngles();
        calculateFilteredAngles();

        unsigned long pidCurrentTime = millis();
        float dt = (pidCurrentTime - pidPreviousTime) / 1000.0; // Convert ms to seconds
        pidPreviousTime = pidCurrentTime;
        
        // --- PID Control Calculations ---
        // Error is the difference between the desired setpoint (0Â°) and the measured roll angle.
        pidError = setpoint - angleData.rollFiltered;
        // Serial.print("PID Angle: ");
        // Serial.println(angleData.rollFiltered);
        // Serial.print("PID ERROR: ");
        // Serial.println(pidError);
        // Integrate the pidError over time
        integral += pidError * dt;
        
        // Calculate the derivative (rate of change of pidError)
        derivative = (pidError - previousError) / dt;
        
        // Compute the PID output
        output = (Kp * pidError) + (Ki * integral) + (Kd * derivative);
        previousError = pidError;
        
        // // Debug print of the PID output
        Serial.print("Kd: ");
        Serial.println(Kd, 6);
        Serial.print("PID ERROR: ");
        Serial.println(pidError, 6);
        // Serial.print("PID INTEGRAL: ");
        // Serial.println(integral, 6);
        Serial.print("PID DERIVITIVE: ");
        Serial.println(derivative, 6);
        Serial.print("PID Output: ");
        Serial.println(output, 6);
        Serial.println();
        
        // Convert the PID output to a motor speed (constrained to PWM range 0-255)
        int motorSpeed = constrain(abs(output), 0, 50);
        motorSpeed = (motorSpeed * 100) / 50; // Convert to percentage
        // --- Motor Control Based on PID Output ---
        // If the output is positive, drive one set of PWM channels;
        // if negative, drive the opposite channels.
        if (output > 0) {
          // Correcting for a tilt that requires forward movement:
          // Activate backward channels to drive the robot forward.
          pwmController.writePWMDutyCycle(motorSpeed);
          pwmController2.writePWMDutyCycle(0);
          pwmController3.writePWMDutyCycle(motorSpeed);
          pwmController4.writePWMDutyCycle(0);
          
          // Serial.println("Moving Forward (Correcting Tilt)");
          // Serial.print("Motor Speed: ");
          // Serial.println(motorSpeed);
        } else if (output < 0) {
          // Correcting for a tilt that requires backward movement:
          // Activate forward channels to drive the robot backward.
          pwmController.writePWMDutyCycle(0);
          pwmController2.writePWMDutyCycle(motorSpeed);
          pwmController3.writePWMDutyCycle(0);
          pwmController4.writePWMDutyCycle(motorSpeed);
          
          // Serial.println("Moving Backward (Correcting Tilt)");
          // Serial.print("Motor Speed: ");
          // Serial.println(motorSpeed);
        } else {
          // If PID output is zero, stop the motors.
          pwmController.writePWMDutyCycle(0);
          pwmController2.writePWMDutyCycle(0);
          pwmController3.writePWMDutyCycle(0);
          pwmController4.writePWMDutyCycle(0);
          
          // Serial.println("No Movement (Balanced)");
        }
      }
    // getAccelData();
    // getGyroData();

    // calculateAngles();
    // calculateFilteredAngles();

    // unsigned long pidCurrentTime = millis();
    // float dt = (pidCurrentTime - pidPreviousTime) / 1000.0; // Convert ms to seconds
    // pidPreviousTime = pidCurrentTime;
    
    // // --- PID Control Calculations ---
    // // Error is the difference between the desired setpoint (0Â°) and the measured roll angle.
    // pidError = setpoint - angleData.rollFiltered;
    // // Serial.println(angleData.rollFiltered);
    // // Integrate the pidError over time
    // integral += pidError * dt;
    
    // // Calculate the derivative (rate of change of pidError)
    // derivative = (pidError - previousError) / dt;
    
    // // Compute the PID output
    // output = Kp * pidError + Ki * integral + Kd * derivative;
    // previousError = pidError;
    
    // // Debug print of the PID output
    // Serial.print("PID ERROR: ");
    // Serial.println(pidError, 6);
    // Serial.print("PID INTEGRAL: ");
    // Serial.println(integral, 6);
    // Serial.print("PID DERIVITIVE: ");
    // Serial.println(derivative, 6);
    // Serial.print("PID Output: ");
    // Serial.println(output, 6);
    
    // // Convert the PID output to a motor speed (constrained to PWM range 0-255)
    // int motorSpeed = constrain(abs(output), 0, 255);
    // motorSpeed = (motorSpeed * 100) / 255; // Convert to percentage
    // // --- Motor Control Based on PID Output ---
    // // If the output is positive, drive one set of PWM channels;
    // // if negative, drive the opposite channels.
    // if (output > 0) {
    //   // Correcting for a tilt that requires forward movement:
    //   // Activate backward channels to drive the robot forward.
    //   pwmController.writePWMDutyCycle(motorSpeed);
    //   pwmController2.writePWMDutyCycle(0);
    //   pwmController3.writePWMDutyCycle(motorSpeed);
    //   pwmController4.writePWMDutyCycle(0);
      
    //   // Serial.println("Moving Forward (Correcting Tilt)");
    //   // Serial.print("Motor Speed: ");
    //   // Serial.println(motorSpeed);
    // } else if (output < 0) {
    //   // Correcting for a tilt that requires backward movement:
    //   // Activate forward channels to drive the robot backward.
    //   pwmController.writePWMDutyCycle(0);
    //   pwmController2.writePWMDutyCycle(motorSpeed);
    //   pwmController3.writePWMDutyCycle(0);
    //   pwmController4.writePWMDutyCycle(motorSpeed);
      
    //   // Serial.println("Moving Backward (Correcting Tilt)");
    //   // Serial.print("Motor Speed: ");
    //   // Serial.println(motorSpeed);
    // } else {
    //   // If PID output is zero, stop the motors.
    //   pwmController.writePWMDutyCycle(0);
    //   pwmController2.writePWMDutyCycle(0);
    //   pwmController3.writePWMDutyCycle(0);
    //   pwmController4.writePWMDutyCycle(0);
      
    //   // Serial.println("No Movement (Balanced)");
    // }

    // printData();

    // sampleFlag = false;
    // missedSamples = 0;

    // Insert logic hear so that everytime sensor is sampled we move the motors accordingly

  // }



  // Retrieve sensor data
  // getGyroData(gyroDataTemp);
  // getAccelData(accelDataTemp);
  // // Calculate raw and filtered angles from sensor data
  // calculateAngles(accelDataTemp, gyroDataTemp);
  // calculateFilteredAngles(gyroDataTemp, accelDataTemp);
  // Debug printing of sensor values
  // Serial.print("\n\n\n");
  // Serial.print("Gyro Data: ");
  // Serial.print(gyroData.gx);
  // Serial.print(", ");
  // Serial.print(gyroData.gy);
  // Serial.print(", ");
  // Serial.println(gyroData.gz);
  
  // Serial.print("Accel Data: ");
  // Serial.print(accelData.ax);
  // Serial.print(", ");
  // Serial.print(accelData.ay);
  // Serial.print(", ");
  // Serial.println(accelData.az);
  
  // Serial.print("Roll (Filtered): ");
  // Serial.println(angleData.rollFiltered, 6);
  
  // Calculate time step (dt)
  // unsigned long pidCurrentTime = millis();
  // float dt = (pidCurrentTime - pidPreviousTime) / 1000.0; // Convert ms to seconds
  // pidPreviousTime = pidCurrentTime;
  
  // // --- PID Control Calculations ---
  // // Error is the difference between the desired setpoint (0Â°) and the measured roll angle.
  // pidError = setpoint - angleData.rollFiltered;
  // // Serial.println(angleData.rollFiltered);
  // // Integrate the pidError over time
  // integral += pidError * dt;
  
  // // Calculate the derivative (rate of change of pidError)
  // derivative = (pidError - previousError) / dt;
  
  // // Compute the PID output
  // output = Kp * pidError + Ki * integral + Kd * derivative;
  // previousError = pidError;
  
  // // Debug print of the PID output
  // Serial.print("PID ERROR: ");
  // Serial.println(pidError, 6);
  // Serial.print("PID INTEGRAL: ");
  // Serial.println(integral, 6);
  // Serial.print("PID DERIVITIVE: ");
  // Serial.println(derivative, 6);
  // Serial.print("PID Output: ");
  // Serial.println(output, 6);
  
  // // Convert the PID output to a motor speed (constrained to PWM range 0-255)
  // int motorSpeed = constrain(abs(output), 0, 255);
  // motorSpeed = (motorSpeed * 100) / 255; // Convert to percentage
  // // --- Motor Control Based on PID Output ---
  // // If the output is positive, drive one set of PWM channels;
  // // if negative, drive the opposite channels.
  // if (output > 0) {
  //   // Correcting for a tilt that requires forward movement:
  //   // Activate backward channels to drive the robot forward.
  //   pwmController.writePWMDutyCycle(motorSpeed);
  //   pwmController2.writePWMDutyCycle(0);
  //   pwmController3.writePWMDutyCycle(motorSpeed);
  //   pwmController4.writePWMDutyCycle(0);
    
  //   // Serial.println("Moving Forward (Correcting Tilt)");
  //   // Serial.print("Motor Speed: ");
  //   // Serial.println(motorSpeed);
  // } else if (output < 0) {
  //   // Correcting for a tilt that requires backward movement:
  //   // Activate forward channels to drive the robot backward.
  //   pwmController.writePWMDutyCycle(0);
  //   pwmController2.writePWMDutyCycle(motorSpeed);
  //   pwmController3.writePWMDutyCycle(0);
  //   pwmController4.writePWMDutyCycle(motorSpeed);
    
  //   // Serial.println("Moving Backward (Correcting Tilt)");
  //   // Serial.print("Motor Speed: ");
  //   // Serial.println(motorSpeed);
  // } else {
  //   // If PID output is zero, stop the motors.
  //   pwmController.writePWMDutyCycle(0);
  //   pwmController2.writePWMDutyCycle(0);
  //   pwmController3.writePWMDutyCycle(0);
  //   pwmController4.writePWMDutyCycle(0);
    
  //   // Serial.println("No Movement (Balanced)");
  // }
  
  // Small delay for loop stability (if needed)
  // delay(10);
}

void stopAllMotors() {
  pwmController.writePWMDutyCycle(0);
  pwmController2.writePWMDutyCycle(0);
  pwmController3.writePWMDutyCycle(0);
  pwmController4.writePWMDutyCycle(0);
}
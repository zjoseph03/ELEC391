
#include <Arduino.h>
#include "include/AccelGyro.h"
#include <mbed.h>
// #include "include/ELEC391PWM.h"
// PWMController pwmController(2); // motor 1 forward
// PWMController pwmController2(3); // motor 1 backward
// PWMController pwmController3(4); // motor 2 forward
// PWMController pwmController4(5); // motor 2 backward

// Low pass filter for sensor readings
float filteredAngle = 0.0;
float filterFactor = 0.4;
float filterIncrement = 0.015;
float filteredDerivitive = 0;

// Code for keyboard input to change kp
// Add these variables at the top with your other global variables
const float kpIncrement = 1.0;  // Amount to change Kp with each keypress
const float kiIncrement = 5.0;
const float kdIncrement = 0.05;

// Motor deadband scaling
const int MOTOR_DEADBAND = 60;  // Minimum PWM before motors move
const float OUTPUT_SCALING = 1.0;  // Scale down the PID output

bool serialCommandReady = false;
String inputString = "";

// PID tuning constants
double kpValues[] = {13, 15,17,20};
int currentKpIndex = 0; 
unsigned long kpTestStartTime;
const unsigned long kpTestDuration = 20000;
double Kp = 1.2;   // Proportional gain
float Ki = 6.15;    // Integral gain
float Kd = 0.0585;    // Derivative gain
float angle;
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
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  pidPreviousTime = millis();
  Kp = kpValues[currentKpIndex];
  kpTestStartTime = millis();

  Kp = 18.0;    // Less aggressive proportional response
  Ki = 0.0;    // Start with zero to avoid windup
  Kd = 0.0;   // Moderate derivative for dampening oscillations
  
  // Kd = kpValues[0];    // Derivative gain

  // Serial.print("Starting test with Kp = ");
  // Serial.println(Kp);
  
  // Set PWM frequency to 500Hz for all motor controllers
  // pwmController.setFrequency(500);
  // pwmController2.setFrequency(500);
  // pwmController3.setFrequency(500);
  // pwmController4.setFrequency(500);
  samplingTicker.attach(mbed::callback(sampleSensors), std::chrono::milliseconds(1000 / samplingFreq));
}

void loop() {
  // Add this at the beginning of your loop
  processSerialInput();

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
    // Kd = 0.0;
    // Ki = 0.0;
  
    // Check if it's time to switch to the next Kp value
    if (currentTime - kpTestStartTime >= kpTestDuration) {
      // Move to next Kp value
      currentKpIndex++;

      
      // Reset integral to prevent accumulated error affecting next test
      integral = 0;


      
      // Check if we've tested all values
      if (currentKpIndex < 4) {
        // Kp = kpValues[currentKpIndex];
        kpTestStartTime = currentTime;
        // Kp = 9.0;   // Proportional gain
        // Kd = 0.549;   // Proportional gain
        // Ki = 0.0;    // Integral gain
        
        // Serial.print("Switching to Kd = ");
        // Serial.println(Kd, 6);
      } else {
        // All tests complete
        //Serial.println("All Kp tests complete");
        // Stop motors
        // stopAllMotors();
        // while(1); // Halt program or implement a restart mechanism
      }
    }
      
      // Kp = 8.0;   // Proportional gain
      if (sampleFlag) {
        sampleFlag = false;
        // dt = 0.01;
        getAccelData();
        getGyroData();

        calculateAngles();
        calculateFilteredAngles();

        unsigned long pidCurrentTime = millis();
        float dt = (float)(pidCurrentTime - pidPreviousTime) / 1000.0; // Convert ms to seconds
        // dt = 0.01;
        pidPreviousTime = pidCurrentTime;
        filteredAngle = filterFactor * angleData.rollFiltered + (1 - filterFactor) * filteredAngle;
        
        // --- PID Control Calculations ---
        // Error is the difference between the desired setpoint (0Â°) and the measured roll angle.
        // pidError = setpoint - angleData.rollFiltered;
        pidError = setpoint - filteredAngle;
        
        // Serial.print("PID Angle: ");
        // Serial.println(angleData.rollFiltered);
        // Serial.print("PID ERROR: ");
        // Serial.println(pidError);
        // Integrate the pidError over time
        integral += pidError * dt;
        
        // Calculate the derivative (rate of change of pidError)
        derivative = (pidError - previousError) / dt;
        // filteredDerivitive = (filterFactor * derivative) + ((1 - filterFactor) * filteredDerivitive);
        // filteredDerivitive = derivative;

        // Compute the PID output
        output = (Kp * pidError) + (Ki * integral) + (Kd * filteredDerivitive);
        previousError = pidError;
        
        // // Debug print of the PID output
        // Serial.print("Kp: ");
        // Serial.println(Kp, 6);
        // Serial.print("PID ERROR: ");
        // Serial.println(pidError, 6);
        // Serial.print("PID INTEGRAL: ");
        // Serial.println(integral, 6);
        // Serial.print("PID DERIVITIVE: ");
        // Serial.println(derivative, 6);
        // // Serial.print("Kp=");
        // // Serial.print(Kp, 6);
        // Serial.print(", Output=");
        // Serial.print(output, 6);
        // Serial.print(", Angle=");
        // Serial.println(angleData.rollFiltered, 6);
        // Serial.println();
        // Serial.println();
        // Serial.println();
        
        
        // Convert the PID output to a motor speed (constrained to PWM range 0-255)
        int motorSpeed = constrain(abs(output), 0, 255);
        
        
        // METHOD 1: USING DEADBAND FOR LINEAR RESPONSE
        // int rawMotorSpeed = abs(output) * OUTPUT_SCALING;
        // int motorSpeed = (rawMotorSpeed < MOTOR_DEADBAND) ? 0 : constrain(rawMotorSpeed, MOTOR_DEADBAND, 255);


        // METHOD 2: Apply exponential curve for more gentle response at small angles
        // float outputScaled = output * OUTPUT_SCALING;
        // int motorSpeed = 0;
        // if (abs(outputScaled) > MOTOR_DEADBAND) {
        //     // Exponential mapping (gentler at small values)
        //     motorSpeed = constrain(
        //         MOTOR_DEADBAND + (255 - MOTOR_DEADBAND) * pow(abs(outputScaled)/255.0, 1.5), 
        //         0, 255);
        // }

        // motorSpeed = ((double)motorSpeed * 100.0) / 255.0; // Convert to percentage
        // Serial.print("Output: ");
        // Serial.println(output);
        // --- Motor Control Based on PID Output ---
        // If the output is positive, drive one set of PWM channels;
        // if negative, drive the opposite channels.
        if (output > 0) {
          // Correcting for a tilt that requires forward movement:
          // Activate backward channels to drive the robot forward.
          analogWrite(2, motorSpeed);
          analogWrite(3, 0);
          analogWrite(4, motorSpeed);
          analogWrite(5, 0);

          
          
          // pwmController.writePWMDutyCycle(motorSpeed);
          // pwmController2.writePWMDutyCycle(0);
          // pwmController3.writePWMDutyCycle(motorSpeed);
          // pwmController4.writePWMDutyCycle(0);
          
          // Serial.println("Moving Forward (Correcting Tilt)");
          // Serial.print("Motor Speed: ");
          // Serial.println(motorSpeed);
        } else if (output < 0) {
          // Correcting for a tilt that requires backward movement:
          // Activate forward channels to drive the robot backward.
          analogWrite(2, 0);
          analogWrite(3, motorSpeed);
          analogWrite(4, 0);
          analogWrite(5, motorSpeed);


          // pwmController.writePWMDutyCycle(0);
          // pwmController2.writePWMDutyCycle(motorSpeed);
          // pwmController3.writePWMDutyCycle(0);
          // pwmController4.writePWMDutyCycle(motorSpeed);
          
          // Serial.println("Moving Backward (Correcting Tilt)");
          // Serial.print("Motor Speed: ");
          // Serial.println(motorSpeed);
        } else {
          // If PID output is zero, stop the motors.
          analogWrite(2, 0);
          analogWrite(3, 0);
          analogWrite(4, 0);
          analogWrite(5, 0);

          // pwmController.writePWMDutyCycle(0);
          // pwmController2.writePWMDutyCycle(0);
          // pwmController3.writePWMDutyCycle(0);
          // pwmController4.writePWMDutyCycle(0);
          
          // Serial.println("No Movement (Balanced)");
        }
      }
}

// void stopAllMotors() {
//   pwmController.writePWMDutyCycle(0);
//   pwmController2.writePWMDutyCycle(0);
//   pwmController3.writePWMDutyCycle(0);
//   pwmController4.writePWMDutyCycle(0);
// }

// Add this function after your loop() function
void processSerialInput() {
  if (Serial.available() > 0) {
    Serial.println("*");

    char inChar = (char)Serial.read();
    
    // Serial.print("Received char code: ");
    // Serial.println((int)inChar);

    // if (inChar == '\n' || inChar == '\r') {
    serialCommandReady = true;
    // } else {
      // Add character to input string
    inputString += inChar;
    Serial.println(inputString);
    // }
  }
  
  if (serialCommandReady) {
    Serial.println("Command Recieved");
    inputString.trim();  // Remove whitespace
    
    // Process commands
    if (inputString.equals("q") || inputString.equals("Q")) {
      Kp += kpIncrement;
      Serial.print("Kp increased to: ");
      Serial.println(Kp, 6);
    }
    else if (inputString.equals("a") || inputString.equals("A")) {
      Kp -= kpIncrement;
      Serial.print("Kp decreased to: ");
      Serial.println(Kp, 6);
    }
    else if (inputString.equals("e") || inputString.equals("E")) {
      Ki += kiIncrement;
      Serial.print("Ki decreased to: ");
      Serial.println(Ki, 6);
    }
    else if (inputString.equals("d") || inputString.equals("D")) {
      Ki -= kiIncrement;
      Serial.print("Ki decreased to: ");
      Serial.println(Ki, 6);
    }
    else if (inputString.equals("r") || inputString.equals("R")) {
      Kd += kdIncrement;
      Serial.print("Kd decreased to: ");
      Serial.println(Kd, 6);
    }
    else if (inputString.equals("f") || inputString.equals("F")) {
      Kd -= kdIncrement;
      Serial.print("Kd decreased to: ");
      Serial.println(Kd, 6);
    }
    else if (inputString.equals("t") || inputString.equals("T")) {
      filterFactor += filterIncrement;
      Serial.print("Filter decreased to: ");
      Serial.println(filterFactor, 6);
    }
    else if (inputString.equals("g") || inputString.equals("G")) {
      filterFactor -= filterIncrement;
      Serial.print("Filter decreased to: ");
      Serial.println(filterFactor, 6);
    }
    // else if (inputString.startsWith("s") || inputString.startsWith("S")) {
    //   // Set Kp to specific value: format "p=25.5"
    //   float newKp = inputString.substring(2).toFloat();
    //   if (newKp != 0.0 || inputString.substring(2).equals("0")) {
    //     Kp = newKp;
    //     Serial.print("Kp set to: ");
    //     Serial.println(Kp, 6);
    //   }
    // }
    else if (inputString.equals("s")) {
      // Print current PID values
      Serial.print(", Output=");
      Serial.print(output, 6);
      Serial.print(", Angle=");
      Serial.print(filteredAngle, 6);
      Serial.print(", Filter=");
      Serial.println(filterFactor, 6);
      Serial.print("Kp=");
      Serial.print(Kp, 6);
      Serial.print(", Ki=");
      Serial.print(Ki, 6);
      Serial.print(", Kd=");
      Serial.println(Kd, 6);
    }
    else if (inputString.length() > 0) {
      Serial.println("Commands: p+ (increase), p- (decrease), p=25.5 (set), pid (show values)");
    }
    
    // Reset for next command
    inputString = "";
    serialCommandReady = false;
  }
}

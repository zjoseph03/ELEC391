
// 7.5 :: 80 :: 0.2

// Kp = 11
// Kd = 0.9
// Ki = 50

#include <Arduino.h>
#include "include/AccelGyro.h"
#include <ArduinoBLE.h>
#include "include/VR30_BLE.h"
#include "include/OLEDDisplay.h"

// Interrupt Includes
#include <mbed.h>
#include <nrf52840.h>
#include <chrono>

// LED Pin definitions
#define LED_KP_UP    A0
#define LED_KP_DOWN  A1  
#define LED_KI_UP    A2
#define LED_KI_DOWN  A3
#define SDA_CLK      A4
#define SDA_DATA     A5
#define LED_KD       A6  // This LED will use patterns for up/down

// Non-blocking LED timing
unsigned long ledOnTime = 0;
const unsigned long LED_FLASH_DURATION = 200;  // 200ms flash duration
bool ledActive = false;

// #include "include/ELEC391PWM.h"
// PWMController pwmController(2); // motor 1 forward
// PWMController pwmController2(3); // motor 1 backward
// PWMController pwmController3(4); // motor 2 forward
// PWMController pwmController4(5); // motor 2 backward

// Declare and initialize BLEController instance
BLEController controllerInstance;
BLEController* BLEController::VR30Controller = nullptr;

// Low pass filter for sensor readings
float filteredAngle = 0.0;
float filterFactor = 1.0;
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
const int samplingFreq = 99; // Sample sensor at 99.84 Hz (every 10ms). Need to experiment to see what sampling freq we can use

// pidPreviousTime is assumed to be defined in one of your libraries; if not, declare it here:
unsigned long pidPreviousTime;

void updateMotorsBLE();
void processSerialInput();
void BLEConnect();
void processBLEPIDFlags();


void setup() {
  Serial.begin(9600);
  // OLEDSetup();
  // while (!Serial);
  
  if (!IMU.begin()) {
    while (1);  // Stop if IMU initialization fails
  }

  Serial.println();
  Serial.println("BLE HID Host");
  Serial.flush();                                                           
  Serial.print("HID_SERVICE ");
  Serial.println(HID_SERVICE);

   // begin initialization
   if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }
  Serial.println("BLE.begin() OK");
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  // Initialize LED pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_KP_UP, OUTPUT);
  pinMode(LED_KP_DOWN, OUTPUT);
  pinMode(LED_KI_UP, OUTPUT);
  pinMode(LED_KI_DOWN, OUTPUT);
  pinMode(LED_KD, OUTPUT);
  
  // Turn all LEDs off initially
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_KP_UP, LOW);
  digitalWrite(LED_KP_DOWN, LOW);
  digitalWrite(LED_KI_UP, LOW);
  digitalWrite(LED_KI_DOWN, LOW);
  digitalWrite(LED_KD, LOW);

  pidPreviousTime = millis();
  Kp = kpValues[currentKpIndex];
  kpTestStartTime = millis();

  Kp = 8.0;    // Less aggressive proportional response
  Kd = 0.9;    // Start with zero to avoid windup
  Ki = 65.0;   // Moderate derivative for dampening oscillations
  
  // Kd = kpValues[0];    // Derivative gain

  // Serial.print("Starting test with Kp = ");
  // Serial.println(Kp);
  
  // Set PWM frequency to 500Hz for all motor controllers
  // pwmController.setFrequency(500);
  // pwmController2.setFrequency(500);
  // pwmController3.setFrequency(500);
  // pwmController4.setFrequency(500);
  BLEController::VR30Controller = &controllerInstance;
  BLE.scan(false);
  samplingTicker.attach(mbed::callback(sampleSensors), std::chrono::milliseconds(1000 / samplingFreq));
}

void loop() {
  // Add this at the beginning of your loop
  // processSerialInput();
  BLE.poll();
  BLEConnect();

  // Arrays to store temporary sensor data
  float accelDataTemp[3];
  float gyroDataTemp[3];
  float dutyCycle;
  float dutyCycleBackwards;
    
  // Kp = 8.0;   // Proportional gain
  if (sampleFlag && BLEController::VR30Controller->controllerConnected) {
    sampleFlag = false;
    // dt = 0.01;
    getAccelData();
    getGyroData();

    calculateAngles();
    calculateFilteredAngles();
    processBLEPIDFlags();

    if (ledActive && (millis() - ledOnTime > LED_FLASH_DURATION)) {
      digitalWrite(LED_KP_UP, LOW);
      digitalWrite(LED_KP_DOWN, LOW);
      digitalWrite(LED_KI_UP, LOW);
      digitalWrite(LED_KI_DOWN, LOW);
      digitalWrite(LED_KD, LOW);
      ledActive = false;
    }

    // unsigned long pidCurrentTime = millis();
    // float dt = (float)(pidCurrentTime - pidPreviousTime) / 1000.0; // Convert ms to seconds
    // dt = 0.01;
    // pidPreviousTime = pidCurrentTime;
    
    // filteredAngle = filterFactor * angleData.rollFiltered + (1 - filterFactor) * filteredAngle;
    filteredAngle = angleData.rollFiltered;
    // Serial.print("Filtered Angle: ");
    // Serial.println(filteredAngle);

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
    integral = constrain(integral, -15, 15);
    
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
    motorSpeed = 255 - motorSpeed;

    
    
    
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
      analogWrite(2, 255);
      analogWrite(3, motorSpeed);
      analogWrite(4, 255);
      analogWrite(5, motorSpeed);

      
      
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
      analogWrite(2, motorSpeed);
      analogWrite(3, 255);
      analogWrite(4, motorSpeed);
      analogWrite(5, 255);


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

void BLEConnect() {
  if (BLEController::VR30Controller && !BLEController::VR30Controller->controllerConnected) {
    // Serial.print("In loop\n");
    BLEController::VR30Controller->BLEInit();
  } else if (BLEController::VR30Controller && !BLEController::VR30Controller->peripheral.connected()) {
    Serial.println("Controller Disconnected");
    digitalWrite(LED_BUILTIN, LOW);

    // Reset flags and clean up
    BLEController::VR30Controller->controllerConnected = false;
    BLEController::VR30Controller->BLEClose();
    
    // Forcefully reset the BLE peripheral object
    BLEController::VR30Controller->peripheral = BLEDevice();
    BLE.end();
    delay(1000);  
    BLE.begin(); 

    // Restart scanning
    Serial.println("Restarting BLE Scan...");
    BLE.scan(false);
  }

  if (BLEController::VR30Controller && BLEController::VR30Controller->controllerConnected && 
    BLEController::VR30Controller->peripheral.connected()) {
    // Turn ON the built-in LED when connected
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void updateMotorsBLE() {
  if (BLEController::VR30Controller->bleControllerUpdated == true) {
    BLEController::VR30Controller->bleControllerUpdated = false;
  } else {
    return;
  }
}

void processBLEPIDFlags() {
  // Process PID flags
  if (pidFlags.KpUpFlag) {
    Kp += kpIncrement;
    Serial.print("Kp+ : ");
    Serial.println(Kp);
    pidFlags.KpUpFlag = false;
    
    // Flash Kp UP LED
    digitalWrite(LED_KP_UP, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  
  if (pidFlags.KpDownFlag) {
    Kp -= kpIncrement;
    Serial.print("Kp- : ");
    Serial.println(Kp);
    pidFlags.KpDownFlag = false;
    
    // Flash Kp DOWN LED
    digitalWrite(LED_KP_DOWN, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  if (pidFlags.KiUpFlag) {
    Ki += kiIncrement;
    Serial.print("Ki+ : ");
    Serial.println(Ki);
    pidFlags.KiUpFlag = false;
    
    // Flash Ki UP LED
    digitalWrite(LED_KI_UP, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  
  if (pidFlags.KiDownFlag) {
    Ki -= kiIncrement;
    Serial.print("Ki- : ");
    Serial.println(Ki);
    pidFlags.KiDownFlag = false;
    
    // Flash Ki DOWN LED
    digitalWrite(LED_KI_DOWN, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }

  if (pidFlags.KdUpFlag) {
    Kd += kdIncrement;
    Serial.print("Kd+ : ");
    Serial.println(Kd);
    pidFlags.KdUpFlag = false;
    
    // Flash Kd LED once for UP
    digitalWrite(LED_KD, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  
  if (pidFlags.KdDownFlag) {
    Kd -= kdIncrement;
    Serial.print("Kd- : ");
    Serial.println(Kd);
    pidFlags.KdDownFlag = false;
    
    // Flash Kd LED twice for DOWN (handled with a pattern)
    digitalWrite(LED_KD, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
}

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

    else if (inputString.equals("s")) {
      // Print current PID values
      Serial.print("Dt = ");
      Serial.print(dt, 6);
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

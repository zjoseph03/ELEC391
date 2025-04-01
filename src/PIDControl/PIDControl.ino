
// 7.5 :: 80 :: 0.2


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
#define LED_FLASH_DURATION 200

// Non-blocking LED timing TODO: get rid of these
unsigned long ledOnTime = 0;
bool ledActive = false;
unsigned long lastDisplayUpdateTime = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 200; // Update at 5Hz

// PWM control with changing frequency
// #include "include/ELEC391PWM.h"
// PWMController pwmController(2); // motor 1 forward
// PWMController pwmController2(3); // motor 1 backward
// PWMController pwmController3(4); // motor 2 forward
// PWMController pwmController4(5); // motor 2 backward

// Declare and initialize BLEController instance
BLEController controllerInstance;
BLEController* BLEController::VR30Controller = nullptr;

// Debug code for incrementing constants on the fly
const float kpIncrement = 1.0;
const float kiIncrement = 5.0;
const float kdIncrement = 0.05;

// Serial input variables 
bool serialCommandReady = false;
String inputString = "";

// PID tuning constants
float Kp = 7.0;    // Less aggressive proportional response
float Kd = 0.4;    // Start with zero to avoid windup
float Ki = 75.0;   // Moderate derivative for dampening oscillations

// Used to update the OLED display
float prevKi;
float prevKp;
float prevKd;

// PID variables
double pidError = 0, previousError = 0;
float integral = 0, derivative = 0;
double output = 0;

mbed::Ticker samplingTicker;
const int samplingFreq = 99; // Sample sensor at 99.84 Hz (every 10ms). Need to experiment to see what sampling freq we can use
float setPoint = 0;  // Target angle (upright position)

bool robotOn = false;
bool prevRobotOn = false;

void updateMotorsBLE();
void processSerialInput();
void BLEConnect();
void processBLEControlFlags();
void turnOffRobot();

void setup() {
  Serial.begin(9600);
  initOLED();
  displayPIDValues(Kp, Ki, Kd, robotOn);
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
  
  // Initialize PWM pins for motor control
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

  Kp = 7.0;    // Less aggressive proportional response
  Kd = 0.4;    // Start with zero to avoid windup
  Ki = 75.0;   // Moderate derivative for dampening oscillations
  
  // Set PWM frequency to 500Hz for all motor controllers
  // pwmController.setFrequency(500);
  // pwmController2.setFrequency(500);
  // pwmController3.setFrequency(500);
  // pwmController4.setFrequency(500);
  
  BLEController::VR30Controller = &controllerInstance;
  BLE.scan(false);
  samplingTicker.attach(mbed::callback(sampleSensors), std::chrono::milliseconds(1000 / samplingFreq));
}

// TODO: Add code to only start when button is pressed 
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

  // Every 200ms check any flags that were set to update the display
  if (millis() - lastDisplayUpdateTime > DISPLAY_UPDATE_INTERVAL) {
    if (Kp != prevKp || Ki != prevKi || Kd != prevKd || prevRobotOn != robotOn)) {
    displayPIDValues(Kp, Ki, Kd, robotOn);
    lastDisplayUpdateTime = millis();
    prevKd = Kd;
    prevKi = Ki;
    prevKp = Kp;
    prevRobotOn = robotOn;
  }
  
  // if (turningData.powerOn == false) {
  //   turnOffRobot();
  // } else {

    // This may turn the robot off too much
    // if (angleData.rollFiltered > 30 || angleData.rollFiltered < -30) {
    //   turningData.powerOn = false;
    // }
    processBLEControlFlags();
    Serial.print("Robot On: ");
    Serial.println(robotOn);
    Serial.print("Integral: ");
    Serial.println(integral);
    Serial.print("Output: ");
    Serial.println(output);

    if (robotOn == false) {
      integral = 0; 
      pidError = 0;
      derivative = 0;
      previousError = 0;
      output = 0;
    } else {
      if (sampleFlag && BLEController::VR30Controller->controllerConnected) {
        sampleFlag = false;
        getAccelData();
        getGyroData();

        calculateAngles();
        calculateFilteredAngles();

        if (ledActive && (millis() - ledOnTime > LED_FLASH_DURATION)) {
          digitalWrite(LED_KP_UP, LOW);
          digitalWrite(LED_KP_DOWN, LOW);
          digitalWrite(LED_KI_UP, LOW);
          digitalWrite(LED_KI_DOWN, LOW);
          digitalWrite(LED_KD, LOW);
          ledActive = false;
        }
        
        // --- PID Control Calculations ---
        // Error is the difference between the desired setpoint (0Â°) and the measured roll angle.
        pidError = setPoint - angleData.rollFiltered;
        
        // Integrate the pidError over time
        integral += pidError * dt;
        
        // Calculate the derivative (rate of change of pidError)
        derivative = (pidError - previousError) / dt;

        // Compute the PID output
        output = (Kp * pidError) + (Ki * integral) + (Kd * derivative);
        previousError = pidError;
        
        // Convert the PID output to a motor speed (constrained to PWM range 0-255)
        int motorSpeed = constrain(abs(output), 0, 255);
        motorSpeed = 255 - motorSpeed;

        if (turningData.turningRight) {
          turningData.rightScaler = 15;
          turningData.leftScaler = -15;
        } else if (turningData.turningLeft) {
          turningData.rightScaler = -15;
          turningData.leftScaler = 15;
        } else {
          turningData.rightScaler = 0.0;
          turningData.leftScaler = 0.0;
        }

        printPIDData(false);

        // --- Motor Control Based on PID Output ---
        // If the output is positive, drive one set of PWM channels;
        // if negative, drive the opposite channels.
        turningData.rightMotorSpeed = constrain(motorSpeed + turningData.rightScaler, 0, 255);
        turningData.leftMotorSpeed = constrain(motorSpeed + turningData.leftScaler, 0, 255);

        if (output > 0) {
          // Correcting for a tilt that requires forward movement:
          // Activate backward channels to drive the robot forward.
          analogWrite(2, 255);
          analogWrite(3, turningData.rightMotorSpeed);
          analogWrite(4, 255);
          analogWrite(5, turningData.leftMotorSpeed);

          
          
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
          analogWrite(2, turningData.rightMotorSpeed);
          analogWrite(3, 255);
          analogWrite(4, turningData.leftMotorSpeed);
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
  }
}

// TODO: Add code to handle BLE disconnects cleanly and to not start accumulating data until desired start
void BLEConnect() {
  if (BLEController::VR30Controller && !BLEController::VR30Controller->controllerConnected) {
    BLEController::VR30Controller->BLEInit();
  } else if (BLEController::VR30Controller && !BLEController::VR30Controller->peripheral.connected()) {
    // TODO: Add code to put robot in balancing mode when controller is disconnected
    Serial.println("Controller Disconnected");
    
    // Choosing to enter balance mode when controller disconnects instead of turning off
    //turningData.powerOn = false;
    pidFlags.balance = true;
    
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

// Function used to process flags triggered by BLE inputs
// NOTE: this is also used for on the fly PID tuning at the moment
void processBLEControlFlags() {
  // Process PID flags
  if (pidFlags.forward) {
    // Kp += kpIncrement;
    setPoint = 3.0;
    // Ki = 40;
    // Serial.println("Forward");
    // Serial.println(setPoint);
    pidFlags.forward = false;
    
    // Flash Kp UP LED
    digitalWrite(LED_KP_UP, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  
  if (pidFlags.backward) {
    // Kp -= kpIncrement;
    setPoint = -3.0;
    // Ki = 40;
    // Serial.println("Backward");
    // Serial.println(setPoint);
    pidFlags.backward = false;
    
    // Flash Kp DOWN LED
    digitalWrite(LED_KP_DOWN, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  if (pidFlags.balance) {
    Serial.println("Balance Mode");
    setPoint = 0;
    pidFlags.balance = false;
    turningData.turningLeft = false;
    turningData.turningRight = false;
    
    // Ki = 50;
    digitalWrite(LED_KI_UP, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  if (pidFlags.x) {
    Kp += kpIncrement;
    Serial.print("Kp+ : ");
    Serial.println(Kp);
    pidFlags.x = false;
    // setPoint = 0.5;
    
    // Flash Ki UP LED
    digitalWrite(LED_KI_UP, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
  
  if (pidFlags.b) {
    Kp -= kpIncrement;
    Serial.print("Kp- : ");
    Serial.println(Kp);
    pidFlags.b = false;
    
  //   // Flash Ki DOWN LED
  //   digitalWrite(LED_KI_DOWN, HIGH);
  //   ledOnTime = millis();
  //   ledActive = true;
  // }

  if (pidFlags.a) {
    // Kd += kdIncrement;
    robotOn = true;
    Serial.print("Kd+ : ");
    Serial.println(Kd);
    pidFlags.a = false;
    
  //   // Flash Kd LED once for UP
  //   digitalWrite(LED_KD, HIGH);
  //   ledOnTime = millis();
  //   ledActive = true;
  // }
  
  if (pidFlags.y) {
    // Kd -= kdIncrement;
    robotOn = false;
    Serial.print("Kd- : ");
    Serial.println(Kd);
    pidFlags.y = false;
    
    // Flash Kd LED twice for DOWN (handled with a pattern)
    digitalWrite(LED_KD, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }

  // Turn left
  if (pidFlags.left) {
    // Ki += kiIncrement;
    // Serial.print("Ki- : ");
    // Serial.println(Ki);
    pidFlags.left = false;
    turningData.turningLeft = true;
    

    // Flash Kd LED twice for DOWN (handled with a pattern)
    digitalWrite(LED_KD, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }

  // Turn right
  if (pidFlags.right) {
    // Ki -= kiIncrement;
    // Serial.println(Ki);

    pidFlags.right = false;
    turningData.turningRight = true;
    
    // Flash Kd LED twice for DOWN (handled with a pattern)
    digitalWrite(LED_KD, HIGH);
    ledOnTime = millis();
    ledActive = true;
  }
}

// DEPRECATED
void processSerialInput() {
  if (Serial.available() > 0) {
    Serial.println("*");

    char inChar = (char)Serial.read();
    
    serialCommandReady = true;

    inputString += inChar;
    Serial.println(inputString);
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
    // else if (inputString.equals("t") || inputString.equals("T")) {
    //   Serial.print("Filter decreased to: ");
    //   Serial.println(filterFactor, 6);
    // }
    // else if (inputString.equals("g") || inputString.equals("G")) {
    //   Serial.print("Filter decreased to: ");
    //   Serial.println(filterFactor, 6);
    // }

    else if (inputString.equals("s")) {
      // Print current PID values
      Serial.print("Dt = ");
      Serial.print(dt, 6);
      Serial.print(", Output=");
      Serial.print(output, 6);
      Serial.print(", Angle=");
      Serial.println(angleData.rollFiltered, 6);
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

float printPIDData(bool printData) {
    // Debug print of the PID output
    if (printData) {
      Serial.print("Kp: ");
      Serial.println(Kp, 6);
      Serial.print("Ki: ");
      Serial.println(Ki, 6);
      Serial.print("Kd: ");
      Serial.println(Kd, 6);
      Serial.print("PID ERROR: ");
      Serial.println(pidError, 6);
      Serial.print("PID INTEGRAL: ");
      Serial.println(integral, 6);
      Serial.print("PID DERIVITIVE: ");
      Serial.println(derivative, 6);
      Serial.print("Setpoint: ");
      Serial.println(setPoint, 6);
      Serial.print(", Output=");
      Serial.print(output, 6);
      Serial.print(", Angle=");
      Serial.println(angleData.rollFiltered, 6);
      Serial.println();
      Serial.println();
      Serial.println();
    }
}

void turnOffRobot() {
  // Turn off the motors
  analogWrite(2, 255);
  analogWrite(3, 255);
  analogWrite(4, 255);
  analogWrite(5, 255);
  derivative = 0;
  integral = 0;
  pidError = 0;
  output = 0;
  previousError = 0;
}

// Summary:
// The vr_30_game and vr_30_mouse hold the current button presses according to packets sent via BLE
// BLEController.gameMode is true when controller is in gamemode, and false when it's not

// TODO's:
// TODO: Try this to control the PWM's and then try this with the real motors
// TODO: Find way to uniquely identify just our device because there may be people with this exact same controller in class 
        // IDEA: Use BLE Bonding to remember previously connected devices, and only connect to them (keys are stored in flash memory)
// TODO: Figure out if the debouncing issue with the B and Power button are going to be a problem or what the cause of it is
// TODO: Remove prints from event handler since it's time critical
// TODO: We don't need to readDescriptors. Purely there for serial prints
// TODO: Create mapping or a table to identify the packets and values stored in the struct with real commands

#include <ArduinoBLE.h>
#include "include/VR30_BLE.h"
#include "include/AccelGyro.h"

// Interrupt includes
#include <mbed.h>
#include <nrf52840.h>
#include <chrono>

// Declare and initialize BLEController instance
BLEController controllerInstance;
BLEController* BLEController::VR30Controller = nullptr;

// Sensor Constants
mbed::Ticker samplingTicker;
const int samplingFreq = 100; // Sample sensor at 100 Hz (every 10ms). Need to experiment to see what sampling freq we can use

void setup() {
  Serial.begin(115200);
  while (!Serial);
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

  // start scanning for peripherals without duplicates
  // Assign instance to static pointer
  BLEController::VR30Controller = &controllerInstance;
  BLE.scan(false);
  samplingTicker.attach(mbed::callback(sampleSensors), std::chrono::milliseconds(1000 / samplingFreq));
}

void loop() {
  // avoid delays of 10ms between polls
  BLE.poll();

  // Connect to correct peripheral and set up event handler to be able to interpret BLE inputs asynchronously
  if (BLEController::VR30Controller && !BLEController::VR30Controller->controllerConnected) {
    // Serial.print("In loop\n");
    BLEController::VR30Controller->BLEInit();
  } else if (BLEController::VR30Controller && !BLEController::VR30Controller->peripheral.connected()) {
    Serial.println("Controller Disconnected");

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

  // Only begin sampling once BLE controller has been connected. 
  // If connected midway, robot has to stall. We can control from interrupt because I2C and other Arduino functions to get sensor data is unpredictable
  if (sampleFlag && BLEController::VR30Controller->controllerConnected) {
    Serial.print("Missed samples: ");
    Serial.println(missedSamples);
    
    getAccelData();
    getGyroData();

    calculateAngles();
    calculateFilteredAngles();

    printData();

    sampleFlag = false;
    missedSamples = 0;

    // Insert logic hear so that everytime sensor is sampled we move the motors accordingly

  }


  // Synchonising motor activity with remote
  // 

  
}

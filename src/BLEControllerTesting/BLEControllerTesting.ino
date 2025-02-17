// Summary:
// The vr_30_game and vr_30_mouse hold the current button presses according to packets sent via BLE
// BLEController.gameMode is true when controller is in gamemode, and false when it's not

// TODO's:
// TODO: Try this to control the PWM's and then try this with the real motors
// TODO: Fix code so that when controller disconnects, we can reconnect it without resetting the code 
// TODO: Find way to uniquely identify just our device because there may be people with this exact same controller in class
// TODO: Figure out if the debouncing issue with the B and Power button are going to be a problem or what the cause of it is
// TODO: Remove prints from event handler since it's time critical
// TODO: We don't need to readDescriptors. Purely there for serial prints
// TODO: Create mapping or a table to identify the packets and values stored in the struct with real commands

#include <ArduinoBLE.h>
#include "include/VR30_BLE.h"

BLEController controllerInstance;
BLEController* BLEController::VR30Controller = nullptr;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(1);
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
}

void loop() {
  // avoid delays of 10ms between polls
  BLE.poll();

  // Connect to correct peripheral and set up event handler to be able to interpret BLE inputs asynchronously
  if (BLEController::VR30Controller && !BLEController::VR30Controller->controllerConnected) {
    // Serial.print("In loop\n");
    BLEController::VR30Controller->BLEInit();
  } else if (BLEController::VR30Controller && !BLEController::VR30Controller->peripheral.connected()) {
    Serial.println("Controller Disconnected\n");
    BLEController::VR30Controller->controllerConnected = false;
    BLEController::VR30Controller->BLEClose();
  }

  
}

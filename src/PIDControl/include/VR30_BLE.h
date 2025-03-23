#include <Arduino.h>
#include <ArduinoBLE.h>

// NOTE: Controller uses MAC randomization so we can't use this to automatically pair with just our controller
const char* targetMacAddress = "DC53106DB062";

static const String HID_SERVICE_alt("1812");
static const String HID_SERVICE("1812");
static const char HID_REPORT_MAP[] = "2A4B";
static const char HID_REPORT[] = "2A4D";

typedef struct __attribute__((__packed__)) {
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t rz;
  uint8_t brake;
  uint8_t gas;
  uint8_t hat:4;
  uint8_t filler:4;
  uint16_t buttons;
} vr_30_game;

// NOTE: If we press the button above the power button it changes to analog mode and changes the Report format
typedef struct __attribute__((__packed__)) {
  uint8_t buttons;
  uint8_t powerSelect;
} vr_30_mouse;

struct PIDFlags {
  bool KpUpFlag = false;
  bool KiUpFlag = false;
  bool KdUpFlag = false;
  bool KpDownFlag = false;
  bool KiDownFlag = false;
  bool KdDownFlag = false;
} pidFlags;


class BLEController {
private:
    
public:
  bool controllerConnected = false;
  bool gameMode;
  BLEDevice peripheral;
  BLEService HIDService;
  BLECharacteristic HIDReportCharacteristic;
  bool bleControllerUpdated = false;
  vr_30_mouse *joy_mouse;
  vr_30_game *joy_game;

  // Global pointer to the robot instance for the static callback
  static BLEController* VR30Controller;

  static void BLEControllerHIDEventHandler(BLEDevice central, BLECharacteristic characteristic) {
    // This is a static callback - we'll need to use a global pointer to access the class instance
    if (VR30Controller) {
      VR30Controller->handleControllerData(characteristic);
    }
  }

  void handleControllerData(BLECharacteristic& characteristic) {
    uint8_t report[32];
    size_t report_len;

    // check if the characteristic supports Notification or Indication
    if (characteristic.canSubscribe()) {
      // Check for new values
      if (characteristic.valueUpdated()) {
        // Serial.print(service_i);
        Serial.print(" HID Service ");
        Serial.print(HIDService.uuid());
        Serial.print(' ');
        // Serial.print(char_i);
        // Serial.print(" Characteristic ");
        Serial.print(characteristic.uuid());
        Serial.print(" valueUpdated ");
        report_len = characteristic.readValue(report, sizeof(report));
        
        // NOTE: For some reason, the power and B button are registering 4 value changes on button presses instead of 2. May be due to sensitivity in polling?
        // May be because of button debounce on those two buttons but they work on keyboard mode, just not on game mode 
        printHexData(report, report_len);
        
        // NOTE: Remove serial prints since this is a time critical event handler.
        if (report_len == sizeof(vr_30_game)) {
          gameMode = true;
          joy_game = (vr_30_game *)report;
          Serial.print("Game Mode Input: ");
          Serial.print("x: "); Serial.print(joy_game->x);
          Serial.print(" y: "); Serial.print(joy_game->y);
          Serial.print(" buttons: 0x"); Serial.print(joy_game->buttons, HEX);
        } else if (report_len == sizeof(vr_30_mouse)) {
          // NOTE: This is not going to work correctly because report changes size depending on the button pressed in mouse mode. 
          gameMode = false;
          joy_mouse = (vr_30_mouse *)report;
          Serial.print("Mouse Mode Input: ");
          Serial.println(joy_mouse->buttons, HEX);
          bleControllerUpdated = true;

          // Add code here for Ki Flags.
          switch(joy_mouse->buttons) {
            case 0xB3:
              pidFlags.KpUpFlag = true;
              break;
            case 0xCD:
              pidFlags.KiUpFlag = true;
              break;
            case 0xE9:
              pidFlags.KdUpFlag = true;
              break;
            case 0xB4:
              pidFlags.KpDownFlag = true;
              break;
            case 0xEA:
              pidFlags.KiDownFlag = true;
              break;
            case 0x40:
              pidFlags.KdDownFlag = true;
              break;
            default:
              break;
          }
        }
        Serial.println();
      }
    }
  }

  void BLEClose () {
    HIDReportCharacteristic.setEventHandler(BLEUpdated, nullptr);
  }

  void BLEInit () {
    // check if a peripheral has been discovered
    BLE.scanForName("MOCUTE-052Fe-AUTO", true);
    peripheral = BLE.available();
    Serial.print("Searching\n");
    Serial.println(peripheral.localName());
    if (peripheral && peripheral.localName() == "MOCUTE-052Fe-AUTO") {
        // discovered a peripheral, print out address, local name, and advertised service
        Serial.print(peripheral.address());
        Serial.print(" '");
        Serial.print(peripheral.localName());
        Serial.print("' ");
        String service;
        
        for (int i = 0; ; i++) {
          service = peripheral.advertisedServiceUuid(i);
          if (service == "") {
            break;
          }
  
          Serial.print(service);
          Serial.println();
          if (service.equals(HID_SERVICE)) {
            Serial.println("Stop scanning");
            BLE.stopScan();
            controllerConnected = true;
            setupPeripheral(peripheral);
            VR30Controller = this;
            // explorePeripheral(peripheral);
  
            // Serial.println("Start scanning");
            // BLE.scan(false);
          }
        }
        Serial.println();
    }
  }

  void setupPeripheral(BLEDevice peripheral) {
    // connect to the peripheral
    Serial.println("Connecting ...");
 
    if (peripheral.connect()) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed to connect!");
      return;
    }
  
    // discover peripheral attributes
    Serial.println("Discovering attributes ...");
    if (peripheral.discoverAttributes()) {
      Serial.println("Attributes discovered");
    } else {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      return;
    }
  
    // read and print device name of peripheral
    Serial.println();
    Serial.print("Device name: ");
    Serial.println(peripheral.deviceName());
    Serial.print("Appearance: 0x");
    Serial.println(peripheral.appearance(), HEX);
    Serial.println();
  
    HIDService = peripheral.service("1812");
    exploreService(HIDService);
    VR30Controller = this;
    
    Serial.println();
  }

  void exploreService(BLEService service) {
    // print the UUID of the service
    Serial.print("Service ");
    Serial.println(service.uuid());
  
    // loop the characteristics of the service and explore each
    for (int i = 0; i < service.characteristicCount(); i++) {
      BLECharacteristic characteristic = service.characteristic(i);
      if (strcmp(characteristic.uuid(), "2a4d") == 0) {
        // Only set up the event handler for the HID Report
        HIDReportCharacteristic = characteristic;
        characteristic.setEventHandler(BLEUpdated, BLEControllerHIDEventHandler);
      }
      exploreCharacteristic(characteristic);
    }
  }
  
  void exploreCharacteristic(BLECharacteristic characteristic) {
    // print the UUID and properties of the characteristic
    Serial.print("\tCharacteristic ");
    Serial.print(characteristic.uuid());
    Serial.print(", properties 0x");
    Serial.print(characteristic.properties(), HEX);
  
    // check if the characteristic is readable
    if (characteristic.canRead()) {
      Serial.print(", readable, ");
      // read the characteristic value
      characteristic.read();
      if (characteristic.valueLength() > 0) {
        // print out the value of the characteristic
        printHexData(characteristic.value(), characteristic.valueLength());
      }
    }
    Serial.println();
  
    // check if the characteristic supports Notification or Indication
    if (characteristic.canSubscribe()) {
      Serial.print("\tSubscribe ");
      if (characteristic.subscribe()) {
        Serial.println("OK");
      } else {
        Serial.println("Fail");
      }
    }
  
    // loop the descriptors of the characteristic and explore each
    for (int i = 0; i < characteristic.descriptorCount(); i++) {
      BLEDescriptor descriptor = characteristic.descriptor(i);
      
      exploreDescriptor(descriptor);
    }
  }
  
  // NOTE: We don't need this function, we can comment it out when we're done debugging anything
  void exploreDescriptor(BLEDescriptor descriptor) {
    // print the UUID of the descriptor
    Serial.print("\t\tDescriptor ");
    Serial.print(descriptor.uuid());
  
    // read the descriptor value
    descriptor.read();
  
    // print out the value of the descriptor
    Serial.print(", value 0x");
    printHexData(descriptor.value(), descriptor.valueLength());
  
    Serial.println();
  }

  void printHexData(const unsigned char data[], int length) {
    for (int i = length - 1; i >= 0; i--) {  // Iterate in reverse
      unsigned char b = data[i];
  
      if (b < 16) {
        Serial.print("0");
      }
  
      Serial.print(b, HEX);
      Serial.print(' ');
    }
  }
};



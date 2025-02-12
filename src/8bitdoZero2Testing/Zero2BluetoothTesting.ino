// MAC Address of our 8bitdo Zero 2 Controller: E417D8EDEC10
#include <ArduinoBLE.h>

// 8bitdo Zero 2 Mac Address
const char* targetMacAddress = "E417D8EDEC10";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central - MAC Address Filter");
  Serial.println("Scanning for target device...");

  // Start scanning for peripherals
  BLE.scan();
}

void loop() {
  // Check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // Get the MAC address of the discovered device
    String deviceAddress = peripheral.address();
    deviceAddress.replace(":", ""); // Remove colons
    deviceAddress.toUpperCase();    // Convert to uppercase

    // Print discovered device for debugging
    Serial.print("Found device: ");
    Serial.print(deviceAddress);
    Serial.print(" - Name: ");
    Serial.println(peripheral.localName());

    // Check if this is our target device
    if (deviceAddress == String(targetMacAddress)) {
      Serial.println("Found target device! Connecting...");
      BLE.stopScan();
      
      if (connectToPeripheral(peripheral)) {
        // Device connected, you can add your control code here
        while (peripheral.connected()) {
          // Add your controller input handling code here
          // For example, reading characteristic values
          delay(100);
        }
      }
      
      // If we lose connection, restart scan
      Serial.println("Disconnected. Restarting scan...");
      BLE.scan();
    }
  }
}

bool connectToPeripheral(BLEDevice& peripheral) {
  if (peripheral.connect()) {
    Serial.println("Connected!");
    
    // Discover peripheral attributes
    if (peripheral.discoverAttributes()) {
      Serial.println("Attributes discovered");
      return true;
    } else {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      return false;
    }
  } else {
    Serial.println("Failed to connect!");
    return false;
  }
}
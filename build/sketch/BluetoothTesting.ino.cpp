#include <Arduino.h>
#line 1 "d:\\Courses\\ELEC391\\BluetoothTesting\\BluetoothTesting.ino"
#include <ArduinoBLE.h>

// Define the pins you want to control
const int LED_PIN = 13;

// Create a BLE service and characteristic
BLEService ledService("180A"); // Custom service
BLEByteCharacteristic ledCharacteristic("2A56", BLERead | BLEWrite); // Custom characteristic

#line 10 "d:\\Courses\\ELEC391\\BluetoothTesting\\BluetoothTesting.ino"
void setup();
#line 41 "d:\\Courses\\ELEC391\\BluetoothTesting\\BluetoothTesting.ino"
void loop();
#line 10 "d:\\Courses\\ELEC391\\BluetoothTesting\\BluetoothTesting.ino"
void setup() {
  // Start serial communication
  Serial.begin(9600);
  while (!Serial);

  
  // Initialize the BLE hardware
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Set the local name for the BLE device
  BLE.setLocalName("ELEC391_A10_Bluetooth");
  BLE.setAdvertisedService(ledService);

  // Add the characteristic to the service
  ledService.addCharacteristic(ledCharacteristic);

  // Add the service
  BLE.addService(ledService);

  // Start advertising
  BLE.advertise();

  // Set the pin mode for the LED pin
  pinMode(LED_PIN, OUTPUT);

  Serial.println("BLE LED Control is ready");
}

void loop() {
  // Listen for BLE peripherals to connect
  BLEDevice central = BLE.central();

  // If a central is connected to the peripheral
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // While the central is connected
    while (central.connected()) {
      // Check if the characteristic value has been written
      if (ledCharacteristic.written()) {
        // Get the value written to the characteristic
        byte value = ledCharacteristic.value();

        // Control the LED based on the value
        if (value == 1) {
          digitalWrite(LED_PIN, HIGH); // Turn on the LED
          Serial.println("LED is ON");
        } else if (value == 0) {
          digitalWrite(LED_PIN, LOW); // Turn off the LED
          Serial.println("LED is OFF");
        }
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

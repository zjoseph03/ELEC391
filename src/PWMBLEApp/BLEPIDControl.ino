#include <Arduino.h>
#include <ArduinoBLE.h>

// Define PID constants and increments
float Kp = 16.0;  // Initial values - adjust as needed
float Ki = 0.0;
float Kd = 0.0;
float kpIncrement = 0.1;
float kiIncrement = 0.01;
float kdIncrement = 0.05;

// BLE service and characteristics
BLEService pidService("000019b1-0000-1000-8000-00805f9b34fb");
BLEFloatCharacteristic kpCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic kiCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic kdCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite | BLENotify);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // BLE setup
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
  }
  
  BLE.setLocalName("A10Arduino");
  
  // Add each characteristic to the service
  pidService.addCharacteristic(kpCharacteristic);
  pidService.addCharacteristic(kiCharacteristic);
  pidService.addCharacteristic(kdCharacteristic);
  
  BLE.addService(pidService);
  Serial.print("Service UUID: ");
  Serial.println(pidService.uuid());
  
  // Set initial values
  updatePidValues();
  
  BLE.advertise();
  Serial.println("BLE device active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();
  
  if (central) {
    Serial.println("Connected to central device");
    digitalWrite(LED_BUILTIN, HIGH);
    kpCharacteristic.writeValue(Kp);
    kiCharacteristic.writeValue(Ki);
    kdCharacteristic.writeValue(Kd);
    
    while (central.connected()) {
      kpCharacteristic.writeValue(Kp);
      // Serial.println("test");
      // kpCharacteristic.writeValue(Kp);
      // Check for Kp changes
      if (kpCharacteristic.written()) {
        float command = kpCharacteristic.value();
        if (command == 1.0) {
          Kp += kpIncrement;
          Serial.print("Kp increased to: ");
        } else { // (command == 0.0) {
          Kp -= kpIncrement;
          Serial.print("Kp decreased to: ");
        }
        Serial.println(Kp);
        kpCharacteristic.writeValue(Kp);  // Send updated value
      }
      
      // Check for Ki changes
      if (kiCharacteristic.written()) {
        float command = kiCharacteristic.value();
        if (command == 1.0) {
          Ki += kiIncrement;
          Serial.print("Ki increased to: ");
        } else { // (command == 0.0) {
          Ki -= kiIncrement;
          Serial.print("Ki decreased to: ");
        }
        Serial.println(Ki);
        kiCharacteristic.writeValue(Ki);  // Send updated value
      }
      
      // Check for Kd changes
      if (kdCharacteristic.written()) {
        float command = kdCharacteristic.value();
        if (command == 1.0) {
          Kd += kdIncrement;
          Serial.print("Kd increased to: ");
        } else { // (command == 0.0) {
          Kd -= kdIncrement;
          Serial.print("Kd decreased to: ");
        }
        Serial.println(Kd);
        kdCharacteristic.writeValue(Kd);  // Send updated value
      }
    }
    
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Disconnected from central");
  }
}

// Function to update all PID values via BLE
void updatePidValues() {
  kpCharacteristic.writeValue(Kp);
  kiCharacteristic.writeValue(Ki);
  kdCharacteristic.writeValue(Kd);
  
  Serial.println("PID Values Updated:");
  Serial.print("Kp = ");
  Serial.println(Kp);
  Serial.print("Ki = ");
  Serial.println(Ki);
  Serial.print("Kd = ");
  Serial.println(Kd);
}
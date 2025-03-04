#include <Arduino.h>
#include <FlashIAP.h>
#include <mbed.h>

// 64 KB is free from being overwritten. 
// Safe flash storage region: 0x000F0000 → 0x00100000 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  Serial.println("STARTING NOW");

  mbed::FlashIAP flash;
  int ret = flash.init();
  if (ret != 0) {
    Serial.println("Flash initialization failed");
    return;
  

  // Get flash memory information
  const uint32_t flash_start = flash.get_flash_start();
  const uint32_t flash_size = flash.get_flash_size();
  const uint32_t flash_end = flash_start + flash_size;

  Serial.print("Flash starts at 0x");
  Serial.println(flash_start, HEX);
  Serial.print("Flash size: 0x");
  Serial.println(flash_size, HEX);
  Serial.print("Flash ends at 0x");
  Serial.println(flash_end, HEX);

  // Calculate last sector information 
  const uint32_t last_addr = flash_end - 1;
  const uint32_t sector_size = flash.get_sector_size(last_addr);
  const uint32_t sector_start = (last_addr / sector_size) * sector_size;

  Serial.print("\nUsing last sector:\n");
  Serial.print("Sector start: 0x");
  Serial.println(sector_start, HEX);
  Serial.print("Sector size: ");
  Serial.println(sector_size);

  // Define data to write
  const uint32_t data_to_write = 0xDEADBEEF;
  const uint32_t write_address = sector_start;  // Start of last sector

  // Check address validity
  if (write_address + sizeof(data_to_write) > flash_end) {
    Serial.println("Address out of range");
    flash.deinit();
    return;
  }

  // Erase the sector first (required before writing)
  Serial.println("\nErasing sector...");
  ret = flash.erase(sector_start, sector_size);
  if (ret != 0) {
    Serial.println("Erase failed");
    flash.deinit();
    return;
  }

  // Write data to flash
  Serial.println("Writing data...");
  ret = flash.program(&data_to_write, write_address, sizeof(data_to_write));
  if (ret != 0) {
    Serial.println("Write failed");
    flash.deinit();
    return;
  }

  flash.deinit();

  // Read back the data
  volatile uint32_t* read_address = (volatile uint32_t*)write_address;
  uint32_t read_data = *read_address;

  Serial.print("\nWritten value: 0x");
  Serial.println(data_to_write, HEX);
  Serial.print("Read value:    0x");
  Serial.println(read_data, HEX);

  if (read_data == data_to_write) {
    Serial.println("Success! Data verification passed!");
  } else {
    Serial.println("Error: Data verification failed!");
  }
}

void loop() {
  // Empty
  delay(1000);
}
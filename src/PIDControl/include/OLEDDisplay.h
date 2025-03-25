#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin (or -1 if not used)
#define OLED_ADDR 0x3C    // I2C address

// Define the display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Function prototypes
void initOLED() {
  Wire.begin();
  // Initialize serial here if needed, or call Serial.begin() in your main file
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Hang if initialization fails
  }
  display.clearDisplay();
  display.display();
}

void displayTestMessage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Hello, Nano 33 BLE!");
  display.display();
}

#endif

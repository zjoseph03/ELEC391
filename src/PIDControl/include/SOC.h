#ifndef SOC_H
#define SOC_H

#include <Adafruit_NeoPixel.h>

const int adcPin = A7;             // ADC input pin for battery voltage measurement
const float scaleFactor = 6.0;     // Scale factor for the voltage divider (10kΩ & 2kΩ)

// Battery voltage limits for interpolation
const float minVoltage = 7.5;      // 0% battery
const float maxVoltage = 11.0;     // 100% battery

// Neopixel settings
#define PIN_NEOPIXEL 11            // Digital pin connected to Neopixel data-in
#define NUMPIXELS 12            // Total number of LEDs in the Neopixel Jewel

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

struct ColorPalette {
  uint32_t red = pixels.Color(255, 0, 0);
  uint32_t green = pixels.Color(0, 255, 0);
  uint32_t blue = pixels.Color(0, 0, 255);
  uint32_t white = pixels.Color(255, 255, 255);
  uint32_t black = pixels.Color(0, 0, 0);
  uint32_t yellow = pixels.Color(255, 255, 0);
  uint32_t purple = pixels.Color(255, 0, 255);
  uint32_t cyan = pixels.Color(0, 255, 255);
  uint32_t orange = pixels.Color(255, 165, 0);
  uint32_t pink = pixels.Color(255, 192, 203);
  uint32_t brown = pixels.Color(165, 42, 42);
  uint32_t lightBlue = pixels.Color(173, 216, 230);
  uint32_t lightGreen = pixels.Color(144, 238, 144);
  uint32_t lightGray = pixels.Color(211, 211, 211);
  uint32_t darkGray = pixels.Color(169, 169, 169);
  uint32_t lightPurple = pixels.Color(221, 160, 221);
  uint32_t lightOrange = pixels.Color(255, 228, 181);
  uint32_t lightPink = pixels.Color(255, 182, 193);
  uint32_t lightBrown = pixels.Color(210, 180, 140);
  uint32_t lightYellow = pixels.Color(255, 255, 224);
  uint32_t lightRed = pixels.Color(255, 160, 122);
  uint32_t lightCyan = pixels.Color(224, 255, 255);
};

// Create an instance of the ColorPalette struct
ColorPalette colors;

void ledRed() {
  // pinMode(PIN_NEOPIXEL, OUTPUT);  // Set Neopixel pin as output
  // pinMode(adcPin, INPUT);       // Set ADC pin as input

  // analogReadResolution(10);        // Set ADC resolution (0-1023)
  // pixels.begin();                  // Initialize the Neopixel strip
  pixels.clear();
  pixels.setBrightness(20);
  pixels.show();

  uint32_t color = pixels.Color(0, 0, 0);

  for (int i = 0; i < 7; i++) {
    pixels.setPixelColor(i, colors.red);
  }
  pixels.show();
}

void ledGreen() {
  pixels.clear();
  pixels.setBrightness(20);
  pixels.show();

  uint32_t color = pixels.Color(0, 0, 0);

  for (int i = 0; i < 7; i++) {
    pixels.setPixelColor(i, colors.green);
  }
  pixels.show();
}

float ledSoc() {
  // Read the ADC value and convert to a measured voltage (0-3.3V)
  int adcValue = analogRead(adcPin);
  float measuredVoltage = (adcValue / 1023.0) * 3.3;
  
  // Calculate the actual battery voltage using the voltage divider's scale factor
  float batteryVoltage = measuredVoltage * scaleFactor;
  
  // Calculate battery percentage using linear interpolation:
  // 7.5V = 0% and 11V = 100%
  float batteryPercentage = ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;
  
  // Clamp the percentage between 0 and 100%
  if (batteryPercentage > 100.0) batteryPercentage = 100.0;
  if (batteryPercentage < 0.0) batteryPercentage = 0.0;

  return batteryPercentage;

  // Print battery voltage and percentage to the Serial Monitor
  // Serial.print("Battery Voltage: ");
  // Serial.print(batteryVoltage, 2);
  // Serial.print(" V | Battery Percentage: ");
  // Serial.print(batteryPercentage, 1);
  // Serial.println(" %");

  // Determine how many LEDs to light based on battery percentage.
  // For example, if batteryPercentage is 50%, then (50/100)*7 = 3.5, so about 3 LEDs should be lit.
//   int litPixels = (int)((batteryPercentage / 100.0) * NUMPIXELS)*(3.3/5);

//   // Clear any previous LED colors
  
  
//   // Set the LED color (using green here; you can choose any color)
//   uint32_t color;
//   switch ((int)batteryPercentage) {
//     case 0 ... 20:
//       color = pixels.Color(255, 0, 0); // Red for low battery
//       break;
//     case 21 ... 50:
//       color = pixels.Color(255, 165, 0); // Orange for medium battery
//       break;
//     case 51 ... 100:
//       color = pixels.Color(0, 255, 0); // Green for high battery
//       break;
//     default:
//       break;
// }

  
//   // Light up the determined number of LEDs (starting from LED 0)
//   for (int i = 0; i < litPixels; i++) {
//     pixels.setPixelColor(i, color);
//   }
//   pixels.show();
}

#endif // SOC_H
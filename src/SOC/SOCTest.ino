#include <Adafruit_NeoPixel.h>

const int adcPin = A0;             // ADC input pin for battery voltage measurement
const float scaleFactor = 6.0;     // Scale factor for the voltage divider (10kΩ & 2kΩ)

// Battery voltage limits for interpolation
const float minVoltage = 7.5;      // 0% battery
const float maxVoltage = 11.0;     // 100% battery

// Neopixel settings
#define PIN_NEOPIXEL 11            // Digital pin connected to Neopixel data-in
#define NUMPIXELS 12            // Total number of LEDs in the Neopixel Jewel

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);        // Set ADC resolution (0-1023)
  pixels.begin();                  // Initialize the Neopixel strip
  pixels.clear();
  pixels.setBrightness(21);
  pixels.show();
}

void loop() {
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

  batteryPercentage = 80;

  // Print battery voltage and percentage to the Serial Monitor
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.print(" V | Battery Percentage: ");
  Serial.print(batteryPercentage, 1);
  Serial.println(" %");

  // Determine how many LEDs to light based on battery percentage.
  // For example, if batteryPercentage is 50%, then (50/100)*7 = 3.5, so about 3 LEDs should be lit.
  int litPixels = (int)((batteryPercentage / 100.0) * NUMPIXELS)*(3.3/5);

  // Clear any previous LED colors
  
  
  // Set the LED color (using green here; you can choose any color)
  uint32_t color = pixels.Color(255, 0, 0);
  
  // Light up the determined number of LEDs (starting from LED 0)
  for (int i = 0; i < litPixels; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();

  delay(1000);  // Update every second
}

const int adcPin = A7;  // ADC input pin
const float scaleFactor = 6.0;  // Scale factor for 10kΩ & 2kΩ voltage divider

// Battery percentage limits
const float minVoltage = 7.5;  // 0% battery
const float maxVoltage = 11.0; // 100% battery

void setup() {
    Serial.begin(115200);
    analogReadResolution(10);  // Set ADC to 10-bit mode (0-1023)
}

void loop() {
    // Read ADC value
    int adcValue = analogRead(adcPin);
    
    // Convert ADC reading to actual voltage
    float voltage = (adcValue / 1023.0) * 3.3;  
    float batteryVoltage = voltage * scaleFactor;

    // Calculate battery percentage using linear interpolation
    float batteryPercentage = ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;

    // Clamp values to 0%-100%
    if (batteryPercentage > 100.0) batteryPercentage = 100.0;
    if (batteryPercentage < 0.0) batteryPercentage = 0.0;

    // Display results
    Serial.print("Battery Voltage: ");
    Serial.print(batteryVoltage, 2);  // Print with 2 decimal places
    Serial.print("V | Battery Percentage: ");
    Serial.print(batteryPercentage, 1); // Print with 1 decimal place
    Serial.println("%");

    delay(1000);  // Wait 1 second before next reading
}

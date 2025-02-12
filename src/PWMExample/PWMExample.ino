// Documentation for mbedOS: https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html
// TODO: Build this class into a seperate header file that can be used by other files freely


#include <Arduino.h>
#include "include/ELEC391PWM.h" // NOTE: FIX THIS SO IT"S NOT DEPENDENT ON MY LOCAL DIRECTORY

#define PWM_PIN_9 3
#define FREQUENCY_1KHZ 1000 // Max freq = 4.294 Ghz
#define ANALOG_INPUT_PIN A0 // Analog input pin to control PWM if needed

// Create PWMController object
PWMController pwmController(2);
PWMController pwmController2(3);
PWMController pwmController3(4);
PWMController pwmController4(5);

void setup() {
  Serial.begin(9600);

  // Set an analog input pin for prototyping
  pinMode(ANALOG_INPUT_PIN, INPUT);
  
  // Example 2: Change frequency from 1Khz to 500Hz and repeat example 1
  pwmController.setFrequency(500);  // Change to 500Hz
  pwmController2.setFrequency(500);
  pwmController3.setFrequency(500);  // Change to 500Hz
  pwmController4.setFrequency(500);
}

void loop() {
    // Example 1: Ramp up duty cycle every second for 10 seconds from 0% to 10%
    // Both Motors moving in the same direction
    for(float duty = 25; duty <= 100; duty += 25) {
        Serial.print("Both Motors Forward at: ");
        Serial.println(duty, 6);
        pwmController.writePWMDutyCycle(duty);
        pwmController3.writePWMDutyCycle(duty);
        delay(2000);
    }

    Serial.println("Setting all PWM's to 0 for 1 second");
    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(0);
    delay(1000);

    // // Motor 1 moving Forwards. Motor 2 moving backwards
    for (float duty = 25; duty <= 100; duty += 25) {
      Serial.print("Motor 1 Forward at 25%\n");
      Serial.print("Motor 2 Backwards at 75%\n");
      pwmController.writePWMDutyCycle(25);
      pwmController4.writePWMDutyCycle(75);
      delay(1000);
    }

    Serial.println("Setting all PWM's to 0 for 1 second");
    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(0);
    delay(1000);

    // Both motors moving backwards
    for(float duty = 25; duty <= 100; duty += 25) {
      Serial.print("Both Motors Backward at: ");
      Serial.println(duty, 6);
      pwmController2.writePWMDutyCycle(duty);
      pwmController4.writePWMDutyCycle(duty);
      delay(2000);
    }

    Serial.println("Setting all PWM's to 0 for 1 second");
    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(0);
    delay(1000);

    // Motor 1 moving backwards. Motor 2 moving Forward
    Serial.print("Motor 1 Backwards at 25%\n");
    Serial.print("Motor 2 Forwards at 75%\n");
    pwmController2.writePWMDutyCycle(25);
    pwmController3.writePWMDutyCycle(75);
    delay(5000);

    Serial.println("Setting all PWM's to 0 for 1 second");
    pwmController.writePWMDutyCycle(0);
    pwmController2.writePWMDutyCycle(0);
    pwmController3.writePWMDutyCycle(0);
    pwmController4.writePWMDutyCycle(0);
    delay(1000);


    
    // // Example 2: Change frequency from 1Khz to 500Hz and repeat example 1
    // pwmController.setFrequency(500);  // Change to 500Hz
    // pwmController2.setFrequency(500);
    // pwmController3.setFrequency(500);  // Change to 500Hz
    // pwmController4.setFrequency(500);
    // delay(10);
}

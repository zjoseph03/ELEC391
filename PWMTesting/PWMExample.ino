// Documentation for mbedOS: https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html
// TODO: Build this class into a seperate header file that can be used by other files freely


#include <Arduino.h>
#include "ELEC391PWM.h"

// Create PWMController object
PWMController pwmController;

void setup() {
  Serial.begin(9600);

  // Set PWM pin as output
  pinMode(PWM_PIN_9, OUTPUT);
  pinMode(ANALOG_INPUT_PIN, INPUT);

  // Set up PWM controller. Default duty cycle == 0%. Default Freq == 500Hz
  pwmController.init(PWM_PIN_9);
  
}

void loop() {
    // Example 1: Ramp up duty cycle every second for 10 seconds from 0% to 10%
    for(float duty = 0; duty <= 100; duty += 10) {
        pwmController.writePWMDutyCycle(PWM_PIN_9, duty);
        delay(1000);
    }
    
    // Example 2: Change frequency from 1Khz to 500Hz and repeat example 1
    pwmController.setFrequency(500);  // Change to 500Hz
    delay(2000);
}

// Documentation for mbedOS: https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html
// TODO: Build this class into a seperate header file that can be used by other files freely


#include <Arduino.h>
#include "include/ELEC391PWM.h" // NOTE: FIX THIS SO IT"S NOT DEPENDENT ON MY LOCAL DIRECTORY

#define PWM_PIN_9 3
#define FREQUENCY_1KHZ 1000 // Max freq = 4.294 Ghz
#define ANALOG_INPUT_PIN A0 // Analog input pin to control PWM if needed

// List to store PWM Duty cycle to RPM mapping for 25, 50, 75, 100% 
// List to store PWM Duty cycle to RPM mapping for 25, 50, 75, 100% 
float dutyCycleList[4] = {15.44, 21.60, 31.82, 100.00};

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

// TODO: Make a look-up table for these values (long term since mapping will change due to alot of factors constantly)
// NOTE: TEST THESE BEFORE DEMO!!!!!
// 25% RPM == 100.5 == 15.44 % PWM
// 50% RPM == 201 == 21.6% RPM
// 75% RPM == 301.5 == 31.82% RPM
// 100% RPM == 402 == 100% RPM

void loop() {
  // // CODE FOR DETERMINING PWM TO RPM
  // pwmController.writePWMDutyCycle(47.66);
  // pwmController2.writePWMDutyCycle(0);
  // delay(8000);

  // //   for(float duty = 10; duty <= 100; duty += 10) {
  //     Serial.print("Both Motors Forward at: ");
  //     Serial.println(duty, 6);
  //     pwmController.writePWMDutyCycle(duty);
  //     pwmController3.writePWMDutyCycle(duty);
  //     delay(8000);
  // }

  // Example 1: Ramp up duty cycle every second for 10 seconds from 0% to 10%
  // Both Motors moving in the same direction
  for(uint32_t duty = 25; duty <= 100; duty += 25) {
      Serial.print("Both Motors Forward at: ");
      Serial.println(duty, 6);
      pwmController.writePWMDutyCycle(dutyCycleList[duty / 25] - 1);
      pwmController3.writePWMDutyCycle(dutyCycleList[duty / 25] - 1);
      delay(8000);
  }



  Serial.println("Setting all PWM's to 0 for 1 second");
  pwmController.writePWMDutyCycle(0);
  pwmController2.writePWMDutyCycle(0);
  pwmController3.writePWMDutyCycle(0);
  pwmController4.writePWMDutyCycle(0);
  delay(2000);

  // Motor 1 moving Forwards. Motor 2 moving backwards
  for (uint32_t duty = 25; duty <= 100; duty += 25) {
    Serial.print("Motor 1 Forward at 25%\n");
    Serial.print("Motor 2 Backwards at 75%\n");
    pwmController.writePWMDutyCycle(dutyCycleList[0]);
    pwmController4.writePWMDutyCycle(dutyCycleList[2]);
    delay(1000);
  }

  Serial.println("Setting all PWM's to 0 for 1 second");
  pwmController.writePWMDutyCycle(0);
  pwmController2.writePWMDutyCycle(0);
  pwmController3.writePWMDutyCycle(0);
  pwmController4.writePWMDutyCycle(0);
  delay(1000);

  // Both motors moving backwards
  for(uint32_t duty = 25; duty <= 100; duty += 25) {
    Serial.print("Both Motors Backward at: ");
    Serial.println(duty, 6);
    pwmController2.writePWMDutyCycle(dutyCycleList[duty / 25] - 1);
    pwmController4.writePWMDutyCycle(dutyCycleList[duty / 25] - 1);
    delay(8000);
  }

  Serial.println("Setting all PWM's to 0 for 1  second");
  pwmController.writePWMDutyCycle(0);
  pwmController2.writePWMDutyCycle(0);
  pwmController3.writePWMDutyCycle(0);
  pwmController4.writePWMDutyCycle(0);
  delay(1000);

  // Motor 1 moving backwards. Motor 2 moving Forward
  Serial.print("Motor 1 Backwards at 25%\n");
  Serial.print("Motor 2 Forwards at 75%\n");
  pwmController2.writePWMDutyCycle(dutyCycleList[0]);
  pwmController3.writePWMDutyCycle(dutyCycleList[2]);
  delay(5000);

  Serial.println("Setting all PWM's to 0 for 1 second");
  pwmController.writePWMDutyCycle(0);
  pwmController2.writePWMDutyCycle(0);
  pwmController3.writePWMDutyCycle(0);
  pwmController4.writePWMDutyCycle(0);
  delay(2000);


    
    // // Example 2: Change frequency from 1Khz to 500Hz and repeat example 1
    // pwmController.setFrequency(500);  // Change to 500Hz
    // pwmController2.setFrequency(500);
    // pwmController3.setFrequency(500);  // Change to 500Hz
    // pwmController4.setFrequency(500);
    // delay(10);
}

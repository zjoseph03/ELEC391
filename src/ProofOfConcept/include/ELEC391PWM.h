// Documentation for mbedOS: https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html

#include <Arduino.h>
#include <mbed.h>

class PWMController {
  private:
    bool isInitialized;
    uint16_t currentDigitalPin;
    float currentDutyCycle;
    static const uint8_t MIN_DUTY_CYCLE = 0;
    static const uint8_t MAX_DUTY_CYCLE = 100;
    mbed::PwmOut* pwm;
    uint32_t currentFrequency;

  // Private default constructor - prevents PWMController pwm;
  PWMController() = delete;
  void checkInitialized() {
      if (!isInitialized) {
          Serial.println("Error: PWM not initialized");
          while(1);
      }
  }

  public:
  PWMController(uint8_t pin) :
    isInitialized(false),
    currentDigitalPin(pin),
    currentDutyCycle(0.0f),
    pwm(nullptr),
    currentFrequency(500) // Default 500Hz
    {
      pwm = new mbed::PwmOut(digitalPinToPinName(pin));
      if (pwm != nullptr) {
          // Set the default freq to 500 Hz for this pin's PWM
          uint32_t period_us = 1000000 / currentFrequency;
          pwm->period_us(period_us);
          pinMode(pin, OUTPUT);
          isInitialized = true;
      } else {
          Serial.println("Failed to initialize PWM on pin " + String(pin));
      }
    }

      

    ~PWMController() {
      if (pwm != nullptr) {
          delete pwm;
      }
    }
     
    void setFrequency(uint32_t freq_hz) {
      if (pwm != nullptr && isInitialized == true) {
          currentFrequency = freq_hz;
          uint32_t period_us = 1000000 / freq_hz;
          pwm->period_us(period_us);
      }
    }

    void writePWMDutyCycle(float dutyCycle) {
      if (!isInitialized) {
          return;
      }
      currentDutyCycle = constrain(dutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE) / MAX_DUTY_CYCLE;
      pwm->write(currentDutyCycle);
    }

    uint32_t getFrequency() const {
      return currentFrequency;
    }

    float getDutyCycle() const {
      return currentDutyCycle * MAX_DUTY_CYCLE;
    }
};

    // DEPRECIATED CODE (mbedOS allows simple way to change frequency so using that now. DO NOT USE THIS CODE ALONGSIDE mbedOS code)
    // void setResolution(uint8_t bits) {
    //     bits = constrain(bits, 2, 16);
    //     analogWriteResolution(PWM_PIN_9, bits);
    //     currentResolution = (1 << bits) - 1;
    // }

    // uint16_t dutyCycleToValue(float dutyCycle) {
    //   // Bound duty cycle to 0.0-100.0%
    //   currentDutyCycle = constrain(dutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE) / MAX_DUTY_CYCLE;
      
    //   // Convert percentage to PWM value with float precision
    //   return (uint16_t)(currentDutyCycle * currentResolution);
    // }

    // void writePWMDutyCycle(uint8_t pin, uint8_t dutyCycle) {
    //     analogWrite(pin, dutyCycleToValue(dutyCycle));
    // }
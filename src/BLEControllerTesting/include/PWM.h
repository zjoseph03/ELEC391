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
        pinMode(pin, OUTPUT);
        isInitialized = true;
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
      } else {
        Serial.println("PWM not properly initialized");
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
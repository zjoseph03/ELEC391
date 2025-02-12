#include "ELEC391PWM.h"
#include <Arduino.h>

class MotorDriver {
    private:
        bool isInitialized;
    // Private default constructor - prevents PWMController pwm;
    MotorDriver() = delete;
    void checkInitialized() {
        if (!isInitialized) {
            Serial.println("Error: Motor Driver not initialized");
            while(1);
        }
    }

    public:
        MotorDriver(uint8_t ForwardsPWMPin, uint8_t BackwardsPWMPin):
        isInitialized(false);
        {
            pwmForward = new PWMController(ForwardsPWMPin);
            pwmBackward = new PWMController(BackwardsPWMPin);
            isInitialized = true;
        }
        
        ~MotorDriver() {
            if (pwmForward != nullptr) {
                delete pwmForward;
            }
            if (pwmBackward != nullptr) {
                delete pwmBackward;
            }
        }

        void forward(float speed) {
            checkInitialized();
            pwmForward->writePWMDutyCycle(speed);
            pwmBackward->writePWMDutyCycle(0);
        }

        void backward(float speed) {
            checkInitialized();
            pwmForward->writePWMDutyCycle(0);
            pwmBackward->writePWMDutyCycle(speed);
        }

        void stop() {
            checkInitialized();
            pwmForward->writePWMDutyCycle(0);
            pwmBackward->writePWMDutyCycle(0);
        }
    
};


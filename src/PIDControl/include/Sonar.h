#ifndef SONAR_H
#define SONAR_H

#define TRIG_PIN D10
#define ECHO_PIN D9

const unsigned long timeout = 30000; // 30ms timeout

unsigned long readSonar(int echoPin) {
  unsigned long startTime = micros();
  
  // Wait for the pin to go HIGH (beginning of pulse)
  while (digitalRead(echoPin) == LOW) {
    if (micros() - startTime > timeout) {
      Serial.println("Timeout 0");
      return 0; // Timeout
    }
  }
  
  // Pin is now HIGH - record the pulse START time
  unsigned long pulseStartTime = micros();
  
  // Wait for the pin to go LOW (end of pulse)
  while (digitalRead(echoPin) == HIGH) {
    if (micros() - pulseStartTime > timeout) {
      Serial.println("Timeout 1");
      return 0; // Timeout 
    }
  }
  
  // Pin is now LOW again - calculate pulse duration
  unsigned long pulseEndTime = micros();
  
  // Return pulse duration in microseconds (NO division needed)
  return pulseEndTime - pulseStartTime;
}

bool runSonar(void) {
    unsigned long duration;
    float distance;

    // Send a 10µs pulse to trigger the sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the time it takes for the echo to return
    // duration = pulseIn(ECHO_PIN, HIGH, 30000);
    duration = readSonar(ECHO_PIN);
    Serial.println(duration);
    // Convert to distance (speed of sound = 343m/s)

    if (duration != 0) {
      distance = (duration * 0.0343) / 2;
      if (distance < 10 && distance > 0) {
        return true;
      } else {
        return false;
      }
      // Serial.print("Distance: ");
      // Serial.println(" cm");

      // Serial.print("Raw duration: ");
      // Serial.println(duration);
    }
    return false; 
}

#endif // SONAR_H
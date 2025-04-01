#define TRIG_PIN D10
#define ECHO_PIN D9

void setup() {
    Serial.begin(9600);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

void loop() {
    long duration;
    float distance;

    // Send a 10µs pulse to trigger the sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(20);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the time it takes for the echo to return
    duration = pulseIn(ECHO_PIN, HIGH, 30000);

    // Convert to distance (speed of sound = 343m/s)
    distance = (duration * 0.0343) / 2;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    Serial.print("Raw duration: ");
    Serial.println(duration);

    delay(500);
}

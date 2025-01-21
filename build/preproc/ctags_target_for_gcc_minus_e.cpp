# 1 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
# 2 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino" 2
# 3 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino" 2


# 4 "d:\\Courses\\ELEC391\\SensorAssignment\\Task4\\Task4_ComplimentaryFilter\\Task4_ComplimentaryFilter.ino"
float k = 0.8; // Complementary filter coefficient
float rollFiltered = 0;
float pitchFiltered = 0;
float lastRollFiltered = 0;
float lastPitchFiltered = 0;
unsigned long previousTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  if (!IMU_BMI270_BMM150.begin()) {
    while (1); // Stop if IMU initialization fails
  }
  previousTime = millis();
}

void loop() {
  float ax, ay, az;
  float ax_g, ay_g, az_g;
  float gx, gy, gz;
  float rollAcc, pitchAcc;
  float deltaT;

  // Time step calculation
  unsigned long currentTime = millis();
  deltaT = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  // Accelerometer Readings
  if (IMU_BMI270_BMM150.accelerationAvailable()) {
    IMU_BMI270_BMM150.readAcceleration(ax, ay, az);

    // Normalize accelerometer values (assuming +/-2g range)
    ax_g = ax / 8192.0;
    ay_g = ay / 8192.0;
    az_g = az / 8192.0;

    // Accelerometer-based angles (in degrees)
    rollAcc = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;
    pitchAcc = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
  }

  // Gyroscope Readings and Complementary Filter
  if (IMU_BMI270_BMM150.gyroscopeAvailable()) {
    IMU_BMI270_BMM150.readGyroscope(gx, gy, gz);

    // Gyroscope angle integration (rate of change to angle)
    float pitchGyro = gz * deltaT;
    float rollGyro = gy * deltaT;

    // Complementary filter application
    rollFiltered = k * (lastRollFiltered + rollGyro) + (1 - k) * rollAcc;
    pitchFiltered = k * (lastPitchFiltered + pitchGyro) + (1 - k) * pitchAcc;

    // Store the current filtered angles for the next iteration
    lastRollFiltered = rollFiltered;
    lastPitchFiltered = pitchFiltered;

    // Output all values in a single comma-separated line:
    // Format: ax,ay,az,rollAcc,pitchAcc,gx,gy,gz,rollGyro,pitchGyro,rollFiltered,pitchFiltered
    Serial.print(ax_g, 6); Serial.print(",");
    Serial.print(ay_g, 6); Serial.print(",");
    Serial.print(az_g, 6); Serial.print(",");
    Serial.print(rollAcc, 6); Serial.print(",");
    Serial.print(pitchAcc, 6); Serial.print(",");
    Serial.print(gx, 6); Serial.print(",");
    Serial.print(gy, 6); Serial.print(",");
    Serial.print(gz, 6); Serial.print(",");
    Serial.print(rollGyro, 6); Serial.print(",");
    Serial.print(pitchGyro, 6); Serial.print(",");
    Serial.print(rollFiltered, 6); Serial.print(",");
    Serial.println(pitchFiltered, 6); Serial.print("\n");

  }
}

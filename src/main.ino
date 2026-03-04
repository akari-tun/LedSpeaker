#include <Arduino.h>

#define MIC_PIN A0
#define SAMPLES 100

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== NOISE FLOOR CALIBRATION TEST ===");
  
  // Perform noise floor calibration
  long sum = 0;
  int val;
  
  Serial.println("Taking ambient noise readings...");
  for (int i = 0; i < SAMPLES; i++) {
    val = analogRead(MIC_PIN);
    sum += val;
    delay(1); // Small delay between readings
  }
  
  float average = (float)sum / SAMPLES;
  int noiseFloor = (int)(average + 20); // Add small buffer
  
  Serial.print("Average ambient: ");
  Serial.println(average);
  Serial.print("Calculated noise floor: ");
  Serial.println(noiseFloor);
  
  delay(500);
}

void loop() {
  Serial.println("Hello from ESP32-C3");
  delay(1000);
}
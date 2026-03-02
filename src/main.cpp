#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("=== SIMPLE TEST ===");
  delay(500);
}

void loop() {
  Serial.println("Hello from ESP32-C3");
  delay(1000);
}
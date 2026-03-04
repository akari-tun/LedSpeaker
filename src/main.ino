#include <Arduino.h>
#include <FastLED.h>

#define MIC_PIN A0
#define SAMPLES 100
#define LED_PIN 4
#define NUM_LEDS 60
#define BRIGHTNESS 50

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== FASTLED + CALIBRATION TEST ===");
  
  // Measure ambient noise floor
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(MIC_PIN);
    delay(1);
  }
  float average = sum / (float)SAMPLES;
  float noiseFloor = average + 10.0;
  
  Serial.print("Noise floor: ");
  Serial.println(noiseFloor);
  
  // Initialize FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Test pattern: all LEDs red
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  
  Serial.println("FastLED initialized - LEDs should be red");
  
  delay(500);
}

void loop() {
  Serial.println("Hello from ESP32-C3");
  delay(1000);
}
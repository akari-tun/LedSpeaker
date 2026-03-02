#include <Arduino.h>
#include <FastLED.h>

#define DATA_PIN 5
#define NUM_LEDS 75

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  Serial.println("HELLO FROM ESP32");
  delay(1000);
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  
  // Test: All LEDs red
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  Serial.println("LEDs should be RED now");
}

void loop() {
  // Blink all LEDs
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  Serial.println("Blinking...");
}
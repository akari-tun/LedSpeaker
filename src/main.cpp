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
  
  Serial.println("=== SIMULATED AUDIO PROCESSING TEST ===");
  
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
  
  // Test pattern: all LEDs green
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  
  Serial.println("Simulated audio processing - LEDs should be green");
  
  delay(500);
}

void loop() {
  // Simulate audio processing without FFT
  int micValue = analogRead(MIC_PIN);
  int ledIndex = map(micValue, 0, 4095, 0, NUM_LEDS-1);
  
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // Light up based on simulated audio level
  for(int i = 0; i <= ledIndex; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  
  Serial.print("Mic: ");
  Serial.print(micValue);
  Serial.print(" -> LED index: ");
  Serial.println(ledIndex);
  
  delay(100);
}
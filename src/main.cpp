#include <Arduino.h>
#include <FastLED.h>
#include "arduinoFFT.h"

#define MIC_PIN A0
#define SAMPLES 100
#define LED_PIN 4
#define NUM_LEDS 60
#define BRIGHTNESS 50

CRGB leds[NUM_LEDS];

// FFT variables
#define FFT_SAMPLES 128
double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFT_SAMPLES, 10000); // 10kHz sampling rate

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== FFT + FASTLED TEST ===");
  
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
  
  // Test pattern: all LEDs blue
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  
  Serial.println("FFT initialized - LEDs should be blue");
  
  delay(500);
}

void loop() {
  // Simple FFT test: just read one sample
  vReal[0] = analogRead(MIC_PIN);
  vImag[0] = 0;
  
  Serial.println("Hello from ESP32-C3");
  delay(1000);
}
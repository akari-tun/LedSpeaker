#include <Arduino.h>
#include <FastLED.h>
#include <arduinoFFT.h>

#define MIC_PIN A0
#define SAMPLES 100
#define LED_PIN 5  // Changed from 4 to 5
#define NUM_LEDS 60
#define BRIGHTNESS 50

// FFT configuration
#define FFT_SAMPLES 128
#define SAMPLING_FREQUENCY 10000

CRGB leds[NUM_LEDS];
double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, FFT_SAMPLES, SAMPLING_FREQUENCY);

float noiseFloor = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== FULL FFT + LED TEST ===");
  
  // Measure ambient noise floor
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(MIC_PIN);
    delay(1);
  }
  float average = sum / (float)SAMPLES;
  noiseFloor = average + 10.0;
  
  Serial.print("Noise floor: ");
  Serial.println(noiseFloor);
  
  // Initialize FastLED with correct pin (5)
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  Serial.println("FFT + FastLED initialized");
  delay(500);
}

void loop() {
  // Read audio samples
  for (int i = 0; i < FFT_SAMPLES; i++) {
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0;
    delayMicroseconds(1000000 / SAMPLING_FREQUENCY);
  }
  
  // Compute FFT - using correct lowercase method names
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
  
  // Get dominant frequency magnitude
  double maxMagnitude = 0;
  for (int i = 2; i < FFT_SAMPLES / 2; i++) { // Skip DC component (i=0,1)
    if (vReal[i] > maxMagnitude) {
      maxMagnitude = vReal[i];
    }
  }
  
  // Map magnitude to brightness (only if above noise floor)
  uint8_t brightness = 0;
  if (maxMagnitude > noiseFloor) {
    brightness = map(maxMagnitude, noiseFloor, 1023, 50, 255);
    brightness = constrain(brightness, 50, 255);
  }
  
  // Set all LEDs to red with audio-controlled brightness
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.setBrightness(brightness);
  FastLED.show();
  
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Max magnitude: ");
    Serial.print(maxMagnitude);
    Serial.print(" | Brightness: ");
    Serial.println(brightness);
    lastPrint = millis();
  }
  
  delay(50);
}
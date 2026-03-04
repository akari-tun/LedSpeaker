#include <Arduino.h>
#include <FastLED.h>
#include <arduinoFFT.h>

#define MIC_PIN A0
#define SAMPLES 100
#define LED_PIN 5
#define NUM_LEDS 60
#define BRIGHTNESS 50

// Optimized FFT configuration for music
#define FFT_SAMPLES 64          // Smaller buffer for faster response
#define SAMPLING_FREQUENCY 8000 // Lower sampling rate for better rhythm detection

CRGB leds[NUM_LEDS];
double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, FFT_SAMPLES, SAMPLING_FREQUENCY);

float noiseFloor = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== MUSIC-OPTIMIZED FFT TEST ===");
  
  // Measure ambient noise floor
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(MIC_PIN);
    delay(1);
  }
  float average = sum / (float)SAMPLES;
  noiseFloor = average + 15.0; // Slightly higher buffer for music
  
  Serial.print("Noise floor: ");
  Serial.println(noiseFloor);
  
  // Initialize FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  Serial.println("Music-optimized FFT ready");
  delay(500);
}

void loop() {
  // Read audio samples quickly
  unsigned long startTime = micros();
  for (int i = 0; i < FFT_SAMPLES; i++) {
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0;
  }
  unsigned long readTime = micros() - startTime;
  
  // Compute FFT
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
  
  // Calculate total energy in bass/mid frequencies (more musical)
  double totalEnergy = 0;
  int freqBins = FFT_SAMPLES / 4; // Focus on lower frequencies (bass/mid)
  for (int i = 2; i < freqBins; i++) {
    totalEnergy += vReal[i];
  }
  totalEnergy /= freqBins; // Average energy
  
  // Map energy to brightness with better scaling
  uint8_t brightness = 0;
  if (totalEnergy > noiseFloor) {
    float normalized = (totalEnergy - noiseFloor) / (1023.0 - noiseFloor);
    brightness = (uint8_t)(normalized * 200 + 50); // Scale 50-255
    brightness = constrain(brightness, 50, 255);
  }
  
  // Set LEDs with smoother response
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.setBrightness(brightness);
  FastLED.show();
  
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) { // Print more frequently for debugging
    Serial.print("Energy: ");
    Serial.print(totalEnergy);
    Serial.print(" | Brightness: ");
    Serial.println(brightness);
    lastPrint = millis();
  }
  
  // Maintain consistent timing
  unsigned long loopTime = micros() - startTime;
  if (loopTime < 20000) { // ~50Hz update rate
    delayMicroseconds(20000 - loopTime);
  }
}
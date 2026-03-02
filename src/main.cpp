#include <Arduino.h>
#include <FastLED.h>
#include <arduinoFFT.h>

#define NUM_ROWS 5
#define NUM_COLS 15
#define FFT_COLS 16
#define NUM_LEDS (NUM_ROWS * NUM_COLS)
#define DATA_PIN 5
#define AUDIO_PIN 0
#define BUTTON_PIN 3
#define SAMPLES 64
#define SAMPLING_DELAY 125

CRGB leds[NUM_LEDS];
ArduinoFFT<double> FFT = ArduinoFFT<double>();
double vReal[SAMPLES];
double vImag[SAMPLES];
float smoothedBands[FFT_COLS] = {0};
float noiseFloor = 20.0;

DEFINE_GRADIENT_PALETTE(soft_gp) {
  0,   0,   0, 255,
  128, 0, 255, 255,
  255, 255, 255, 255
};
CRGBPalette16 softPalette = soft_gp;

int currentMode = 0;
bool lastButtonState = HIGH;

int getLedIndex(int row, int col) {
  int panel = col / 5;
  int localCol = col % 5;
  int localRow = row;
  if (panel == 0) panel = 2;
  else if (panel == 2) panel = 0;
  int rotatedRow = 4 - localCol;
  int rotatedCol = localRow;
  int indexInPanel;
  if (rotatedCol % 2 == 0) {
    indexInPanel = rotatedCol * 5 + rotatedRow;
  } else {
    indexInPanel = rotatedCol * 5 + (4 - rotatedRow);
  }
  return panel * 25 + indexInPanel;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(AUDIO_PIN, ANALOG);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  FastLED.clear();
  FastLED.show();
  
  // Calibrate: measure ambient noise 20 times
  Serial.println("Calibrating noise floor...");
  double noiseSum = 0;
  for (int i = 0; i < 20; i++) {
    double maxVal = 0;
    for (int j = 0; j < SAMPLES; j++) {
      vReal[j] = analogRead(AUDIO_PIN) * 0.6;
      vImag[j] = 0;
      delayMicroseconds(SAMPLING_DELAY);
    }
    FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLES);
    for (int k = 1; k < SAMPLES/2; k++) {
      if (vReal[k] > maxVal) maxVal = vReal[k];
    }
    noiseSum += maxVal;
    Serial.printf("Sample %d: %.1f\n", i, maxVal);
  }
  noiseFloor = (noiseSum / 20) + 15;
  Serial.printf("Noise floor set to: %.1f\n", noiseFloor);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    currentMode = (currentMode + 1) % 5;
    Serial.printf("Mode: %d\n", currentMode);
    delay(200);
  }
  lastButtonState = buttonState;

  if (currentMode == 2) {
    FastLED.clear();
    FastLED.show();
    delay(50);
    return;
  }

  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(AUDIO_PIN) * 0.6;
    vImag[i] = 0;
    delayMicroseconds(SAMPLING_DELAY);
  }

  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  int bandValues[FFT_COLS] = {0};
  int bandsPerBand = (SAMPLES / 2) / FFT_COLS;

  for (int band = 0; band < FFT_COLS; band++) {
    double sum = 0;
    for (int i = band * bandsPerBand; i < (band + 1) * bandsPerBand; i++) {
      sum += vReal[i];
    }
    double rawValue = sum / bandsPerBand;
    rawValue = max(rawValue - noiseFloor, 0.0);
    float mapped = (currentMode <= 1)
                     ? map(rawValue, 0, 300, 0, NUM_ROWS)
                     : map(rawValue, 0, 800, 0, NUM_COLS);
    float smoothingFactor = 0.7;
    smoothedBands[band] = smoothingFactor * smoothedBands[band] + (1 - smoothingFactor) * mapped;
    bandValues[band] = constrain((int)smoothedBands[band], 0, (currentMode <= 1) ? NUM_ROWS : NUM_COLS);
  }

  FastLED.clear();

  if (currentMode == 0 || currentMode == 1) {
    for (int col = 0; col < NUM_COLS; col++) {
      for (int row = 0; row < bandValues[col + 1]; row++) {
        int idx = getLedIndex(row, col);
        uint8_t heat = map(row, 0, NUM_ROWS - 1, 50, 150);
        CRGB color = (currentMode == 0) ? HeatColor(heat) : ColorFromPalette(softPalette, heat);
        color.nscale8_video(150);
        leds[idx] = color;
      }
    }
  } else if (currentMode == 3 || currentMode == 4) {
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < bandValues[row + 1]; col++) {
        int idx = getLedIndex(row, col);
        uint8_t heat = map(col, 0, NUM_COLS - 1, 80, 150);
        CRGB color = (currentMode == 3) ? HeatColor(heat) : ColorFromPalette(softPalette, heat);
        color.nscale8_video(150);
        leds[idx] = color;
      }
    }
  }

  FastLED.show();
}
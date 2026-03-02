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

int currentMode = 0; // Modes: 0=V-Fire, 1=V-Soft, 2=Off, 3=H-Fire, 4=H-Soft
bool lastButtonState = HIGH;
float noiseFloor = 20.0;  // Will be calibrated at startup
bool calibrating = true;

// Define a soft camera-friendly gradient palette
DEFINE_GRADIENT_PALETTE(soft_gp) {
  0,   0,   0, 255,    // Deep blue
  128, 0, 255, 255,    // Cyan
  255, 255, 255, 255   // White
};
CRGBPalette16 softPalette = soft_gp;

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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(AUDIO_PIN, ANALOG);
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);  // 50% brightness
  FastLED.clear();
  FastLED.show();
  
  // Calibrate noise floor (read ambient noise for ~2 seconds)
  Serial.println("Calibrating... keep quiet!");
  double noiseSum = 0;
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < SAMPLES; j++) {
      vReal[j] = analogRead(AUDIO_PIN) * 0.6;
      vImag[j] = 0;
      delayMicroseconds(SAMPLING_DELAY);
    }
    FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLES);
    
    double maxVal = 0;
    for (int k = 0; k < SAMPLES/2; k++) maxVal = max(maxVal, vReal[k]);
    noiseSum += maxVal;
  }
  noiseFloor = (noiseSum / 100) + 10;  // Add 10 as buffer
  Serial.printf("Noise floor calibrated: %.1f\n", noiseFloor);
  calibrating = false;
}

void loop() {
  // Handle button press to switch modes
  bool buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    currentMode = (currentMode + 1) % 5; // Cycle through 0-4
    delay(200); // debounce
  }
  lastButtonState = buttonState;

  if (currentMode == 2) {
    FastLED.clear();
    FastLED.show();
    return;
  }

  // Read audio samples
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(AUDIO_PIN) * 0.6;
    //Serial.printf("%d", vReal[i]);
    vImag[i] = 0;
    delayMicroseconds(SAMPLING_DELAY);
  }

  //Serial.println("---------------------");

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
    rawValue = max(rawValue - noiseFloor, 0.0); // calibrated noise floor

    float mapped = (currentMode <= 1)
                     ? map(rawValue, 0, 300, 0, NUM_ROWS)
                     : map(rawValue, 0, 800, 0, NUM_COLS);

    float smoothingFactor = 0.7;
    smoothedBands[band] = smoothingFactor * smoothedBands[band] + (1 - smoothingFactor) * mapped;
    bandValues[band] = constrain((int)smoothedBands[band], 0, (currentMode <= 1) ? NUM_ROWS : NUM_COLS);
  }

  FastLED.clear();

  if (currentMode == 0 || currentMode == 1) {
    // Vertical bar modes
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
    // Horizontal bar modes
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
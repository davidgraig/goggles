#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define MIC_PIN A9
#define NOISE 10
#define SAMPLE_WINDOW 50

#define AMPLITUDE_THRESHOLD_LOW 40
#define AMPLITUDE_THRESHOLD_MED 100
#define FALLOFF 4

const int numPixels = 32;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, LED_PIN, NEO_GRB + NEO_KHZ800);
const int randomPixelIndexMax = (numPixels / 8);
const int minAmplitude = 1;

int colorAmplitudes[numPixels];

void setup() {
  memset(colorAmplitudes, 0, sizeof(colorAmplitudes));
  strip.begin();
  strip.show();
}

void loop() {
  double volts = getVolts();
  int colorAmplitude = getColorAmplitude(volts);
  uint32_t color = getColor(colorAmplitude);
  updateAmplitudes(colorAmplitude);
  drawStrip(color);
}

void updateAmplitudes(int amplitude)
{
  for (int i = 0; i < numPixels; i++)
  {
    int amplitude = colorAmplitudes[i];
    int cappedFalloff = max(minAmplitude, amplitude - FALLOFF);
    colorAmplitudes[i] = cappedFalloff;
  }
  
  for (int i = 0; i < randomPixelIndexMax * 2; i++) {
    int offset = i * randomPixelIndexMax;
    int offsetMax = offset + 4; 
    int index = (int) random(offset, offsetMax);
    colorAmplitudes[index] = max(colorAmplitudes[index], amplitude);
  }
}

void drawStrip(uint32_t color)
{
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, getWheelColor(colorAmplitudes[i]));
  }  
  strip.show();
}

int getColorAmplitude(double volts)
{
  return (int) (volts * 256);
}

double getVolts()
{
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  while (millis() - startMillis < SAMPLE_WINDOW)
  {
    unsigned int sample = analogRead(MIC_PIN);
    if (sample < 1024)
    {
       if (sample > signalMax)
       {
          signalMax = sample;
       }
       else if (sample < signalMin)
       {
          signalMin = sample;
       }
    }
  }

  double signalDiff = (signalMax - signalMin) * 1.0;
  double volts = signalDiff / 1024;
  return volts;
}

uint32_t getColor(int colorAmplitude) {
  int leftover = 255 - colorAmplitude;
  return strip.Color(0, 0, colorAmplitude);
}

uint32_t getWheelColor(int amplitude) {
  int x = amplitude / 6;
  if (amplitude < AMPLITUDE_THRESHOLD_LOW) {
   return strip.Color(x, 0, amplitude);
  } else if (amplitude < AMPLITUDE_THRESHOLD_MED) {
   return strip.Color(0, amplitude, x);
  } else {
    return strip.Color(amplitude, x, 0);
  }
}

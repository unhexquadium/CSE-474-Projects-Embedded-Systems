/*
   Jacob Chong 1232393
   Kalvin Hallesy 1750416
   Lab5, part 4
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "SPI.h"
#include "ILI9341_t3.h"

// For the Adafruit shield, these are the default.
#define TFT_DC  9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;

// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=249,132
AudioInputAnalog         adc1;           //xy=251,179
AudioOutputAnalog        dac1;           //xy=551,183
AudioAnalyzeFFT1024      fft1024_1;      //xy=560,110
// Connect either the live input or synthesized sine wave
//AudioConnection          patchCord1(adc1, fft1024_1);
//AudioConnection          patchCord2(adc1, dac1);
AudioConnection          patchCord1(sine1, fft1024_1);
AudioConnection          patchCord2(sine1, dac1);
// GUItool: end automatically generated code

int f;
float newLogBin[16];
float oldLogBin[16];
int w;
int counter;
int t;
int a, b;

//AudioControlSGTL5000 audioShield;

void setup() {
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield and set the output volume.
  //  audioShield.enable();
  //  audioShield.inputSelect(myInput);
  //  audioShield.volume(0.5);

  // Configure the window algorithm to use
  fft1024_1.windowFunction(AudioWindowHanning1024);
  //fft1024_1.windowFunction(NULL);
  // Create a synthetic sine wave, for testing
  // To use this, edit the connections above
  sine1.amplitude(0.8);
  sine1.frequency(f);

  f = 20;
  w = 20;
  a = 0;
  b = 0;
  counter = 0;
  t = millis();

  tft.setTextSize(1);
  tft.setRotation(3);
  tft.setCursor(0, 230);
  tft.println(10);
  tft.setCursor(60, 230);
  tft.println(100);
  tft.setCursor(140, 230);
  tft.println(1000);
  tft.setCursor(240, 230);
  tft.println(10000);

  tft.setRotation(1);
  tft.drawLine(320, 20, 0, 20, ILI9341_WHITE);
}


void loop() {

  if (fft1024_1.available()) {
    counter ++;
    int newT = millis();
    if ( newT - t > 1000) {
      Serial.println(counter);
      t = newT;
      counter = 0;
    }

    // log approximation
    // last iteration will call fft1024_1.read(395, 523), where 523 is greater than 512,
    // but as the library says, if the second value is greater than 511, the function
    // automatically changes it to 511.
    for (int i = 0; i < 16; i++) {
      b = a + pow(2, (i / 2));
      newLogBin[i] = fft1024_1.read(a, b);
      a = b + 1;
    }

    for (int i = 0; i < 16; i++) {
      tft.fillRect(320 - (i * 20), 21, w, (int) (oldLogBin[i] * 200), ILI9341_BLACK);
    }

    for (int i = 0; i < 16; i++) {
      tft.fillRect(320 - (i * 20), 21, w, (int) (newLogBin[i] * 200), ILI9341_YELLOW);
    }

    for (int i = 0; i < 16; i++) {
      oldLogBin[i] = newLogBin[i];
    }

    if (f >= 20000) {
      f = 20;
    } else {
      f += 5;
    }
    sine1.frequency(f);
  }
}



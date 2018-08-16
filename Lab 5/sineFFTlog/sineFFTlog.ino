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


float logBin[16];
int a, b;

//AudioControlSGTL5000 audioShield;

void setup() {
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
  sine1.frequency(1034.007);

  a = 0;
  b = 0;
}


void loop() {
  if (fft1024_1.available()) {

    // log approximation
    // last iteration will call fft1024_1.read(395, 523), where 523 is greater than 512,
    // but as the library says, if the second value is greater than 511, the function
    // automatically changes it to 511.
    for (int i = 0; i < 16; i++) {
      b = a + pow(2, (i / 2));
      logBin[i] = fft1024_1.read(a, b);
      a = b + 1;
    }

    Serial.print("FFT: ");
    for (int i = 0; i < 16; i++) {
      if (logBin[i] >= 0.01) {
        Serial.print(logBin[i]);
        Serial.print(" ");
      } else {
        Serial.print("  -  "); // don't print "0.00"
      }
    }
    Serial.println();
  }




}



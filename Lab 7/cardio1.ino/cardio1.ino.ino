/*
   Jacob Chong 1232393
   Kalvin Hallesy 1750416
   Lab7, part 1
*/

#include "SPI.h"
#include "ILI9341_t3.h"
#include <XPT2046_Touchscreen.h>

#define TFT_DC  9
#define TFT_CS 10
#define CS_PIN 4


XPT2046_Touchscreen ts(CS_PIN);

// For the Adafruit shield, these are the default.
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

int oldValue;
int adcValue;
int pixel;
boolean onOff;
boolean ifPressed;

#define PDB_CONFIG    (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
                       | PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

#define INPUT_PIN     8


// 48 MHz / 128 / 10 / 1 Hz = 37500
#define PDB_PERIOD (F_BUS / 128 / 10 / 250)

void setup() {
  pinMode(INPUT_PIN, INPUT);
  analogReadResolution(12);
  tft.begin();
  tft.fillScreen(ILI9341_WHITE);
  adcValue = 0;
  pixel = 0;
  oldValue = 0;

  // Enable PDB clock
  SIM_SCGC6 |= SIM_SCGC6_PDB;
  // Timer period
  PDB0_MOD = PDB_PERIOD;
  // Interrupt delay
  PDB0_IDLY = 0;
  // PDB0_CH0DLY0 = 0;
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;
  // Software trigger (reset and restart counter)
  PDB0_SC |= PDB_SC_SWTRIG;
  // Enable interrupt request
  NVIC_ENABLE_IRQ(IRQ_PDB);

  tft.setRotation(1);
  drawGrid();
}

void loop() {

}

void pdb_isr() {
  adcValue = analogRead(INPUT_PIN);
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK; // (also clears interrupt flag)

  adcValue /= (4096 / 240);

  boolean ifTouched = ts.touched();
  if (!ifPressed && ifTouched) {
    onOff = !onOff;
    ifPressed = true;
  }

  if (!ifTouched) {
    ifPressed = false;
  }

  if (onOff) {
    if (pixel == 0) {
      drawGrid();
      tft.drawPixel(pixel, 240 - adcValue, ILI9341_BLACK);
    } else {
      tft.drawLine(pixel - 1, 240 - oldValue, pixel, 240 - adcValue, ILI9341_BLACK);
    }


    oldValue = adcValue;
    pixel++;
    pixel = pixel % 320;
  } else {
    pixel = 0;
  }
}

void drawGrid() {
  tft.fillScreen(ILI9341_WHITE);
  for (int i = 1; i < 32; i++) {
    if (i % 5 == 0) {
      tft.drawLine(i * 10 + 1, 0, i * 10 + 1, 240, ILI9341_RED);
      tft.drawLine(i * 10 - 1, 0, i * 10 - 1, 240, ILI9341_RED);
    }
    tft.drawLine(i * 10, 0, i * 10, 240, ILI9341_RED);
  }
  for (int i = 1; i < 24; i++) {
    if (i % 5 == 0) {
      tft.drawLine(0, i * 10 + 1, 320, i * 10 + 1, ILI9341_RED);
      tft.drawLine(0, i * 10 - 1, 320, i * 10 - 1, ILI9341_RED);
    }
    tft.drawLine(0, i * 10, 320, i * 10, ILI9341_RED);
  }
}


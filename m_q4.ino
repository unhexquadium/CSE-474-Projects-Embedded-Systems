//Kalvin Hallesy
//EE 474 Midterm q4
//This code makes a pdb timer (part b) and uses it to sample an external buffer signal through the ADC and calculate it's frequency (part c) and either RMS voltage for AC inputs or DC Voltage for DC inputs
//It then displays these values on a NHD‐0216K3Z utilizing SPI


//numbers for moving average
#define FREQ_NUM          30
#define VOL_NUM           5000



#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
                    | PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

// pdb period to determine sampling rate for pdb timer
#define PDB_PERIOD (F_BUS / 128 / 10 / 10000)

//period for refresh of display
#define DIS_PER   60000

//defining pin numbers
#define SLAVE_SELECT       10
#define BUFF_INPUT         18

#include <SPI.h>

//defining variables
int samples;
int volData;
float vol;
float preVol;
float cutOff;
float curTime;
float prevTime;
float intTime;
int freq;
int freqMA[FREQ_NUM];
int freqCount;
int freqTotal;
int volCount;
int freqAvg;
float voltages[VOL_NUM];
float VRmsTotal;
float vRms;
int displayCount;
int isDC;

void setup() {
  //setting pin modes
  pinMode(BUFF_INPUT, INPUT);
  pinMode(SLAVE_SELECT, OUTPUT);

  SPI.begin();
  Serial.begin(9600);

  samples = 0;
  freqCount = 0;
  volCount = 0;
  freqTotal = 0;
  VRmsTotal = 0.0;
  cutOff = 0.1; //sets slightly higher than 0 minimum amplitude for signal to be considered AC or else noise would give off frequency values for a 0V DC input
  curTime = 0.0;
  prevTime = 0.0;
  freqAvg = 0;
  vRms = 0.0;
  displayCount = 0;
  isDC = 0;

  //This is the initialization for the timer, part b
  // Enable PDB clock
  SIM_SCGC6 |= SIM_SCGC6_PDB;
  // Timer period
  PDB0_MOD = PDB_PERIOD;
  // Interrupt delay
  PDB0_IDLY = 0;
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;
  // Software trigger (reset and restart counter)
  PDB0_SC |= PDB_SC_SWTRIG;
  // Enable interrupt request
  NVIC_ENABLE_IRQ(IRQ_PDB);


  // displays "VOLT: " and "FREQ: " at the beginning
  displayLabels();
}

void loop() {
  // getting actual voltage value in range of -6.0V ~ +6.0V
  if (freqCount > FREQ_NUM - 1) {
    freqCount = 0;
  }
  if (volCount > VOL_NUM - 1) {
    volCount = 0;
  }
  if (displayCount > DIS_PER) {

    //calculates root mean squart (RMS) value for voltage since voltage fluctuations of AC voltage would not be feasible for a human to read, this will return the correct DC voltage in the case of DC anyway
    VRmsTotal = 0.0;
    for (int i = 0; i < VOL_NUM; i++) {
      VRmsTotal += (voltages[i] * voltages[i]);
    }
    vRms = sqrt(VRmsTotal / VOL_NUM);

    // calculating moving average of frequency
    freqTotal = 0;
    for (int i = 0; i < FREQ_NUM; i++) {
      freqTotal += freqMA[i];
    }
    freqAvg = freqTotal / FREQ_NUM;

    //prints to serial monitor what should be read on display at given time
    Serial.print("VOLT: "); Serial.print(vRms); Serial.println("V");
    Serial.print("FREQ: "); Serial.print(freqAvg); Serial.println("Hz");

    //writes the voltage and frequency to the display
    displayVoltage(vRms);
    displayFrequency(freqAvg);
    
    
    displayCount = 0;
  }

  vol = (volData / 1023.0 * 3.3 * 4.0) - 6.0; //equation for extracting the original input voltage value 
  voltages[volCount] = vol;

  //updates frequency period only if there is a transition from underneath cutOff voltage to over cutOff voltage
  if (preVol < cutOff && vol > cutOff) {
    curTime = samples; //using out timer sample amount as a time base instead of micros() according to midterm instructions
    intTime = curTime - prevTime;
    prevTime = curTime;
    intTime /= pow(10, 6);
    freq = (int) 1.0 / intTime / 100;
    freqMA[freqCount] = freq;
    isDC = 0;
  } else { //if the above if statement is not triggered, the signal is DC and therefore the frequency values input should be set to 0
    isDC++;

    if (isDC > 50000){
      freqMA[freqCount] = 0;
    }
  }

  //updates all counts for moving average
  preVol = vol;
  freqCount ++;
  volCount ++;
  displayCount ++;
  //displayVol();
}

//dispalys FREQ and VOLT labels and units statically on display
void displayLabels() {
  digitalWrite(SLAVE_SELECT, LOW);
  
  // displaying "VOLT: "
  setCursorPosition(0x00);
  SPI.transfer(0x56);
  moveCursorRight();
  SPI.transfer(0x4F);
  moveCursorRight();
  SPI.transfer(0x4C);
  moveCursorRight();
  SPI.transfer(0x54);
  moveCursorRight();
  SPI.transfer(0x3A);

  //displaying unit V
  setCursorPosition(0x0E);
  SPI.transfer(0x56);

  // displaying "FREQ: "
  setCursorPosition(0x40);
  SPI.transfer(0x46);
  moveCursorRight();
  SPI.transfer(0x52);
  moveCursorRight();
  SPI.transfer(0x45);
  moveCursorRight();
  SPI.transfer(0x51);
  moveCursorRight();
  SPI.transfer(0x3A);

  //displaying unit Hz
  setCursorPosition(0x4E);
  SPI.transfer('H');
  moveCursorRight();
  SPI.transfer('z');

  digitalWrite(SLAVE_SELECT, HIGH);
}

//displays the frequency value in 4 digits by backspace deleting the 4 spots and writing the new frequency, therefore preserving the labels and units
void displayFrequency(int freq) {

  // slave select pin to low, starts writing.
  digitalWrite(SLAVE_SELECT, LOW);
  int freqDis = freq;
  int disDigit;
  
  setCursorPosition(0x4E);
  backSpace();
  backSpace();
  backSpace();
  backSpace();

  setCursorPosition(0x4D);
  while (freqDis) {
    disDigit = freqDis % 10;
    freqDis /= 10; 
    SPI.transfer(char(disDigit));
    moveCursorLeft();
  }
  
  digitalWrite(SLAVE_SELECT, HIGH);
}

//displays the voltage value in 3 digits + decimal point and sign by backspace deleting the 5 spots and writing the new voltage, therefore preserving the labels and units
void displayVoltage(float voltage) {
  int volDis = voltage*10;
  int disDigit;
  
  digitalWrite(SLAVE_SELECT, LOW);
  
  setCursorPosition(0x0E);
  backSpace();
  backSpace();
  backSpace();
  backSpace();
  backSpace();

  if(voltage <0){
    SPI.transfer('-');
  }
  moveCursorRight();

  //calculations for extracting the appropriate digits to use in the display
  disDigit = volDis-volDis%100;
  volDis = volDis-disDigit;
  disDigit /= 100;
  SPI.transfer(char(disDigit));
  moveCursorRight();
  SPI.transfer('.');
  moveCursorRight();
  disDigit = volDis-volDis%10;
  volDis = volDis-disDigit;
  disDigit /= 10;
  SPI.transfer(char(disDigit));
  moveCursorRight();
  SPI.transfer(char(volDis));
  
  digitalWrite(SLAVE_SELECT, HIGH);
}

//this is the interrupt so that we can sample the voltage value on the BUFF_INPUT at precise timer intervals
void pdb_isr() {
  volData = analogRead(BUFF_INPUT);
  samples++;
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK; // (also clears interrupt flag)
}

// the folloring are just general commands that are commonly used in this code on the display, their names are the same as their functions
void moveCursorRight() {
  SPI.transfer(0xFE);
  SPI.transfer(0x4A);
}

void moveCursorLeft() {
  SPI.transfer(0xFE);
  SPI.transfer(0x49);
}

void setCursorPosition(int posi) {
  SPI.transfer(0xFE);
  SPI.transfer(0x45);
  SPI.transfer(posi);
}

void backSpace() {
  SPI.transfer(0xFE);
  SPI.transfer(0x4E);
}



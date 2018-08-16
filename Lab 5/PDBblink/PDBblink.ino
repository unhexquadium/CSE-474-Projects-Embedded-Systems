/*
   Jacob Chong 1232393
   Kalvin Hallesy 1750416
   Lab5, part 2
*/

uint8_t ledOn = 0;

#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
                    | PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

// 48 MHz / 128 / 10 / 1 Hz = 37500
#define PDB_PERIOD (F_BUS / 128 / 10 / 120)

void setup() {
  pinMode(13, OUTPUT);

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
}

void loop() {
  // put your main code here, to run repeatedly:

}

void pdb_isr() {
  digitalWrite(13, (ledOn = !ledOn));// invert the value of the LED each interrupt
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK; // (also clears interrupt flag)
}






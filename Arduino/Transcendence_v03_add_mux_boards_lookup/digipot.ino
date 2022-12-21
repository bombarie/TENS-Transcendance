#include <SPI.h>

void initDigiPot() {
  // setup digital pot
  pinMode(CS_PIN, OUTPUT);  // set the CS_PIN as an output:
  SPI.begin();              // initialize SPI:

  DigitalPotWrite(BOTH_POT_SHUTDOWN, BOTH_POT_Dn);
}

void updateDigiPot() {
  DigitalPotWrite(POT0_SEL, POT0_Dn);  //set the resistance of POT0
  DigitalPotWrite(POT1_SEL, POT1_Dn);  //set the resistance of POT1
}



//Function Declaration
void DigitalPotTransfer(int cmd, int value);  //send the command and the resistance value through SPI

void DigitalPotWrite(int cmd, int val) {
  //  SPI.beginTransaction(SPISettings(SCK, MSBFIRST, SPI_MODE0));
  // constrain input value within 0 - 255
  val = constrain(val, 0, 255);
  // set the CS pin to low to select the chip:
  digitalWrite(CS_PIN, LOW);
  // send the command and value via SPI:
  SPI.transfer(cmd);
  SPI.transfer(val);
  // Set the CS pin high to execute the command:
  digitalWrite(CS_PIN, HIGH);
  //  SPI.endTransaction();
}

/*
 * EEPROM Clear
 *
 * Sets all of the bytes of the I2C EEPROM to 0.
 * 
 * This example code is in the public domain.
 */

#include <AT24Cxx.h>
#define i2c_address 0x50

void setup() {
  
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
    // Initilaize using AT24CXX(i2c_address, size of eeprom in KB).
    AT24Cxx eep(i2c_address, 32);
 
  /***
    Iterate through each byte of the EEPROM storage.
  ***/

  Serial.println("Please Wait this may take a while depending upon you EEPROM size.");
  Serial.print("Size of EEPROM:");
  Serial.println(eep.length());
  Serial.println("KB");
  for (int i = 0 ; i < eep.length() ; i++) {
    eep.update(i, 0x00);
  }

  // Print Msg When Completed.
  Serial.println("EEPROM Erase completed.");
}

void loop() {
  /** Empty loop. **/
}

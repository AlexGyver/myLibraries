/* 
 * File:   AT24Cxx.h
 * Author: Manjunath CV
 *
 * Created on February 16, 2016, 12:19 AM
 */

#ifndef AT24CXX_H
#define AT24CXX_H

#include <Arduino.h>

class AT24Cxx {
public:
    AT24Cxx(uint8_t i2c_address);
    AT24Cxx(uint8_t i2c_address, uint8_t eeprom_size);
    uint8_t read(uint16_t address);
	int readInt(uint16_t address);
	uint32_t readLong(uint16_t address);	
    void write(uint16_t address, uint8_t value);
	void writeInt(uint16_t address, int value);
	void writeLong(uint16_t address, uint32_t value);	
    void update(uint16_t address, uint8_t value);
	  uint16_t length(void);
    //void get();
    //void put();
    //AT24Cxx(const AT24Cxx& orig);
    //virtual ~AT24Cxx();

protected:
    uint8_t eeprom_size;
	uint8_t i2c_address;
};

#endif /* AT24CXX_H */


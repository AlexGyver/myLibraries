/*
* File:   AT24Cxx.cpp
* Author: Manjunath CV
*
* Created on February 16, 2016, 12:19 AM
*/

#include <Wire.h>
#include "AT24Cxx.h"

AT24Cxx::AT24Cxx(uint8_t i2c_address)
{
	Wire.begin();
	this->i2c_address = i2c_address;
}

AT24Cxx::AT24Cxx(uint8_t i2c_address, uint8_t eeprom_size)
{
	Wire.begin();
	this->i2c_address = i2c_address;
	this->eeprom_size = eeprom_size;
}

uint8_t AT24Cxx::read(uint16_t address)
{
	uint8_t first,second,data;
	Wire.beginTransmission(i2c_address);
	
	first = highByte(address);
	second = lowByte(address);
	
	Wire.write(first);      //First Word Address
	Wire.write(second);      //Second Word Address
	
	Wire.endTransmission();
	delay(10);

	Wire.requestFrom(i2c_address, 1);
	delay(10);
	
	data = Wire.read();
	delay(10);
	
	return data;
}

void AT24Cxx::write(uint16_t address, uint8_t value)
{
	uint8_t first,second;
	Wire.beginTransmission(i2c_address);

	first = highByte(address);
	second = lowByte(address);

	Wire.write(first);      //First Word Address
	Wire.write(second);      //Second Word Address

	Wire.write(value);     

	delay(10);

	Wire.endTransmission();
	delay(10);
}

void AT24Cxx::writeLong(uint16_t address, uint32_t value) {	
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);

     //Write the 4 bytes into the eeprom memory.
    write(address, four);
    write(address + 1, three);
    write(address + 2, two);
    write(address + 3, one);
}

uint32_t AT24Cxx::readLong(uint16_t address) {
	//Read the 4 bytes from the eeprom memory.
    long four = read(address);
    long three = read(address + 1);
    long two = read(address + 2);
    long one = read(address + 3);

    //Return the recomposed long by using bitshift.
    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void AT24Cxx::writeInt(uint16_t address, int value) {	
	byte two = (value & 0xFF);
	byte one = ((value >> 8) & 0xFF);

     //Write the 4 bytes into the eeprom memory.
    write(address, two);
    write(address + 1, one);
}

int AT24Cxx::readInt(uint16_t address) {
	//Read the 4 bytes from the eeprom memory.
    uint16_t two = read(address);
    uint16_t one = read(address + 1);

    //Return the recomposed long by using bitshift.
    return (((two << 0) & 0xFF) + ((one << 8) & 0xFFFF));
}

void AT24Cxx::update(uint16_t address, uint8_t value)
{
	uint8_t first,second,data;
	Wire.beginTransmission(i2c_address);

	first = highByte(address);
	second = lowByte(address);

	Wire.write(first);      //First Word Address
	Wire.write(second);      //Second Word Address

	Wire.endTransmission();
	delay(10);

	Wire.requestFrom(i2c_address, 1);
	delay(10);

	data = Wire.read();	
	delay(10);
	
	/* Checking Value */
	if (data != value)
	{
		/* Writing Value */
		Wire.beginTransmission(i2c_address);

		first = highByte(address);
		second = lowByte(address);

		Wire.write(first);      //First Word Address
		Wire.write(second);      //Second Word Address

		Wire.write(value);

		delay(10);

		Wire.endTransmission();
		delay(10);	
	}
}

uint16_t AT24Cxx::length()
{
	return eeprom_size * 1024;	
}


/*
AT24Cxx::AT24Cxx(const AT24Cxx& orig) {
}

AT24Cxx::~AT24Cxx() {
}
*/

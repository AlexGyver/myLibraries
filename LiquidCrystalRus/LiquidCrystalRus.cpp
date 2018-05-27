/* This is the implementation of LiquidCrystalRus library.
 * 
 * (C) 2011 Artem Borisovskiy, bytefu@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "LineDriver.h"
#include "LiquidCrystalExt.h"
#include "LiquidCrystalRus.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// except 0401 --> 0xa2 = Ё, 0451 --> 0xb5 = ё
const unsigned char utf_recode[] PROGMEM =
{
  0x41,0xa0,0x42,0xa1,0xe0,0x45,0xa3,0xa4,0xa5,0xa6,0x4b,0xa7,0x4d,0x48,0x4f,
  0xa8,0x50,0x43,0x54,0xa9,0xaa,0x58,0xe1,0xab,0xac,0xe2,0xad,0xae,0x62,0xaf,0xb0,0xb1,
  0x61,0xb2,0xb3,0xb4,0xe3,0x65,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0x6f,
  0xbe,0x70,0x63,0xbf,0x79,0xe4,0x78,0xe5,0xc0,0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7
};


LiquidCrystalRus::LiquidCrystalRus(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, LineDriver *line_driver):
  LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7, line_driver)
{
}


LiquidCrystalRus::LiquidCrystalRus(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, LineDriver *line_driver):
  LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7, line_driver)
{
}


LiquidCrystalRus::LiquidCrystalRus(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, LineDriver *line_driver):
  LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, line_driver)
{
}


LiquidCrystalRus::LiquidCrystalRus(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, LineDriver *line_driver):
  LiquidCrystal(rs, enable, d0, d1, d2, d3, line_driver)
{
}

#if defined(ARDUINO) && ARDUINO >= 100
size_t LiquidCrystalRus::write(uint8_t data)
#else
void LiquidCrystalRus::write(uint8_t data)
#endif
{
  uint8_t out_char = data;

  if (data >= 0x80) // UTF-8 handling
  {
    if (data >= 0xc0)
      utf_hi_char = data - 0xd0;
    else
    {
      data &= 0x3f;

      if (!utf_hi_char && data == 1)
        out_char = 0xa2; // Ё
      else if (utf_hi_char == 1 && data == 0x11)
        out_char = 0xb5; // ё
      else
        out_char = pgm_read_byte_near(utf_recode + data + (utf_hi_char << 6) - 0x10);

      #if defined(ARDUINO) && ARDUINO >= 100
      return
      #endif
      LiquidCrystal::write(out_char);
    }
  }
  else
    #if defined(ARDUINO) && ARDUINO >= 100
    return
    #endif
    LiquidCrystal::write(out_char);
}

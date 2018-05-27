/* This is header file for LiquidCrystalRus library which adds support
 * for cyrillic characters to LiquidCrystal library for Arduino.
 * It's been originally developed by Ilya Danilov
 * (http://www.blogger.com/profile/11932466603117832284),
 * and was rewritten to support modern versions of LiquidCrystal.
 *
 * See the article about LCDs based on HD44780 at http://robocraft.ru/blog/arduino/503.html
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

#ifndef LIQUIDCRYSTALRUS_H
#define LIQUIDCRYSTALRUS_H

#include <inttypes.h>
#include "LineDriver.h"
#include "LiquidCrystalExt.h"
/* You'll need to write
 *   #include "LineDriver.h"
 *   #include "LiquidCrystalExt.h"
 * in your sketch.
 */

class LiquidCrystalRus: public LiquidCrystal
{
public:
  LiquidCrystalRus(uint8_t rs, uint8_t enable,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, LineDriver *line_driver = 0);

  LiquidCrystalRus(uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, LineDriver *line_driver = 0);

  LiquidCrystalRus(uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, LineDriver *line_driver = 0);

  LiquidCrystalRus(uint8_t rs, uint8_t enable,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, LineDriver *line_driver = 0);
  
  #if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(uint8_t data);
  #else
  virtual void write(uint8_t data);
  #endif

private:
  uint8_t utf_hi_char; // UTF-8 high part
};

#endif // LIQUIDCRYSTALRUS_H

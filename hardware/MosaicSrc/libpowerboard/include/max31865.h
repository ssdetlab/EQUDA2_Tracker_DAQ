/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */

#ifndef MAX31865_H
#define MAX31865_H

#include "sc18is602.h"
#include <stdint.h>

class MAX31865 {
private:
  // Register list
  enum { REG_Configuration = 0x00, REG_RTD_MSB = 0x01, REG_RTD_LSB = 0x02, REG_WRITE = 0x80 };

  // Configuration register bits
  enum {
    CFG_FREQ_50Hz   = 0x01,
    CFG_FAULT_CLEAR = 0x02,
    CFG_3WIRE       = 0x10,
    CFG_1SHORT      = 0x20,
    CFG_CONV_AUTO   = 0x40,
    CFG_Vbias_ON    = 0x80
  };

public:
  MAX31865(SC18IS602 *spi, uint8_t slave);
  void     writeRegister(uint8_t reg, uint8_t data);
  uint8_t  readRegister(uint8_t reg);
  void     configure(uint8_t cfg = CFG_Vbias_ON | CFG_CONV_AUTO | CFG_FAULT_CLEAR | CFG_FREQ_50Hz);
  uint16_t getRTD();

private:
  SC18IS602 *spiBus;
  uint8_t    spiSlave;
};

#endif // MAX31865_H

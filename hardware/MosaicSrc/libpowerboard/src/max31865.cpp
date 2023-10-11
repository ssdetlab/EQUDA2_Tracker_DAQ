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
#include "max31865.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

MAX31865::MAX31865(SC18IS602 *spi, uint8_t slave)
{
  spiBus   = spi;
  spiSlave = slave;
}

void MAX31865::writeRegister(uint8_t reg, uint8_t data)
{
  uint8_t buffer[2];

  buffer[0] = reg | REG_WRITE;
  buffer[1] = data;

  spiBus->spiWrite(spiSlave, 2, buffer);
}

uint8_t MAX31865::readRegister(uint8_t reg)
{
  uint8_t buffer[2];

  buffer[0] = reg;
  buffer[1] = 0x00; // dummy value.

  spiBus->spiWrite(spiSlave, 2, buffer);
  spiBus->spiReadBuffer(2, buffer);

  return buffer[1];
}

void MAX31865::configure(uint8_t cfg) { writeRegister(REG_Configuration, cfg); }

uint16_t MAX31865::getRTD()
{
  uint16_t resH = readRegister(REG_RTD_MSB);
  uint16_t resL = readRegister(REG_RTD_LSB);

  return ((resH & 0xff) << 8) | (resL & 0xff);
}

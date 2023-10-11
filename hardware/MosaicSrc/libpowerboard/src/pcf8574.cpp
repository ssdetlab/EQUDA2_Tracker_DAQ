/*
 * Copyright (C) 2015
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2015.
 *
 */
#include "pcf8574.h"
#include <stdio.h>
#include <stdlib.h>

PCF8574::PCF8574(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

void PCF8574::write(uint8_t b)
{
  // Write Control register content
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(b, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

uint8_t PCF8574::read()
{
  uint32_t r;

  // Read Control register content
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  i2cBus->addRead(&r, I2Cbus::RWF_dontAck | I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
  // printf("PCF8574::read return 0x%x\n", r);
  return (r & 0xff);
}

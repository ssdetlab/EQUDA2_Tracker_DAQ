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
#include "ltc2635.h"
#include <stdio.h>
#include <stdlib.h>

LTC2635::LTC2635(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

void LTC2635::write(uint8_t cmd, uint8_t add, uint16_t data)
{
  add &= 0x0f;

  uint16_t d =
      data
      << 4; // data alignment for LTC2635-12
            //	uint16_t d = data<<6;				// data alignment for LTC2635-10
            //	uint16_t d = data<<8;				// data alignment for LTC2635-8

  // Write command and data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(cmd | add);
  i2cBus->addWriteData(d >> 8);
  i2cBus->addWriteData(d & 0xff, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

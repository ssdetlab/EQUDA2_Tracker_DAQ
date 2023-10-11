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
#include "ad5254.h"
#include "mexception.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

AD5254::AD5254(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

void AD5254::write(uint8_t cmd, uint8_t ch, uint8_t data)
{
  ch &= 0x03;

  // Write command and data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(cmd | ch);
  i2cBus->addWriteData(data, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

// two bytes version for quick commands
void AD5254::write(uint8_t cmd, uint8_t ch)
{
  ch &= 0x03;

  // Write command and data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(cmd | ch, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

uint8_t AD5254::read(uint8_t cmd, uint8_t add)
{
  uint32_t r;

  // dummy write to set address
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(cmd | add);

  // read
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  i2cBus->addRead(&r, I2Cbus::RWF_stop | I2Cbus::RWF_dontAck);

  // send commands packet
  i2cBus->execute();
  return (r & 0xff);
}

/*
        Wait for the completition of current operation
*/
void AD5254::ackPolling()
{
  const int maxTry = 500;
  int       i;

  usleep(200);
  for (i = 0; i < maxTry; i++) {
    try {
      i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
      i2cBus->addWriteData(CMD_NOP, I2Cbus::RWF_stop);
      i2cBus->execute();
      break;
    }
    catch (MIPBusErrorWrite &) {
      // wait and retry
      usleep(200);
    }
  }

  if (i >= maxTry) throw MIPBusErrorWrite("AD5254::ackPolling timeout");
}

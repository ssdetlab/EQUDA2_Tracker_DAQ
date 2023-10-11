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
#include "max5419.h"
#include <stdio.h>
#include <stdlib.h>

MAX5419::MAX5419(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

void MAX5419::setRDAC(uint8_t b)
{
  // Write volatile register content
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(CMD_VREG);
  i2cBus->addWriteData(b, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

void MAX5419::setNVREG(uint8_t b)
{
  // Write non volatile register content
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(CMD_NVREG);
  i2cBus->addWriteData(b, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

void MAX5419::restoreRDAC()
{
  // Move data from non volatile register to volatile register
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(CMD_NVREGxVREG, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

void MAX5419::storeRDAC()
{
  // Move data from volatile register to non volatile register
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(CMD_VREGxNVREG, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

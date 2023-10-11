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
#include "ad7997.h"
#include <stdio.h>
#include <stdlib.h>

AD7997::AD7997(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

/*
        Single byte write operation
*/
void AD7997::write(uint8_t address, uint8_t data)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(address);
  i2cBus->addWriteData(data, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

/*
        2 byte write operation
*/
void AD7997::write(uint8_t address, uint16_t data)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(address);
  i2cBus->addWriteData(data >> 8);
  i2cBus->addWriteData(data & 0xff, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

void AD7997::addWriteAddressPointer(uint8_t address)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(address, I2Cbus::RWF_stop);
}

/*
        Single byte read
*/
void AD7997::read(uint8_t address, uint8_t *data)
{
  uint32_t r;

  // Write the address pointer
  addWriteAddressPointer(address);

  // Read data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  i2cBus->addRead(&r, I2Cbus::RWF_dontAck | I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
  *data = r & 0xff;
}

/*
        2 byte read
*/
void AD7997::read(uint8_t address, uint16_t *data)
{
  uint32_t rh, rl;

  // Write the address pointer
  addWriteAddressPointer(address);

  // Read data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  i2cBus->addRead(&rh);
  i2cBus->addRead(&rl, I2Cbus::RWF_dontAck | I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
  *data = ((rh << 8) & 0xff00) | (rl & 0xff);
}

/*
        Configure the channels to read
*/
void AD7997::setConfiguration(uint8_t chMap)
{
  uint16_t data = (uint16_t)chMap << 4;

  write(CMD_NOP | REG_Configuration, (uint16_t)(data | CFG_FLTR));
}

void AD7997::convert(int nch, uint16_t *result)
{
  uint32_t rh[8], rl[8];

  if (nch > 8) nch = 8;

  // start sequential convertion and select the read register
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(CMD_CONV_SEQ | REG_ConvertionReult);

  // Read data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  for (int i = 0; i < nch; i++) {
    i2cBus->addRead(rh + i);
    i2cBus->addRead(rl + i, (i == nch - 1) ? (I2Cbus::RWF_dontAck | I2Cbus::RWF_stop) : 0);
  }
  i2cBus->execute();

  for (int i = 0; i < nch; i++)
    *result++ = ((rh[i] & 0xff) << 8) | (rl[i] & 0xff);
}

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
#include "ADC128D818.h"
#include "mexception.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ADC128D818::ADC128D818(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

/*
        Single byte write operation
*/
void ADC128D818::write(uint8_t address, uint8_t data)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(address);
  i2cBus->addWriteData(data, I2Cbus::RWF_stop);

  // send commands packet
  i2cBus->execute();
}

void ADC128D818::addWriteAddressPointer(uint8_t address)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(address, I2Cbus::RWF_stop);
}

/*
        Single byte read
*/
void ADC128D818::read(uint8_t address, uint8_t *data)
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
void ADC128D818::read(uint8_t address, uint16_t *data)
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
void ADC128D818::setConfiguration()
{
  write(REG_Configuration, 0); // enter in configuration mode
  write(REG_AdvancedConfiguration, ADV_CFG_Mode1);
  write(REG_ConvertionRate, 1);        // set Continuous Convertion Mode
  write(REG_Configuration, CFG_Start); // start converting
}

void ADC128D818::getConfiguration(uint8_t *data) { read(REG_Configuration, data); }

void ADC128D818::convert(uint16_t *result)
{
  for (int i = 0; i < 8; i++) {
    read(REG_ChannelReadings + i, result);
    result++;
  }
}

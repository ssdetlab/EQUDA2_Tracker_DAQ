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
#include "sc18is602.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_LEN 200

SC18IS602::SC18IS602(I2Cbus *busPtr, uint8_t address) : I2Cslave(busPtr, address) {}

/*
        Configure the bridge
*/
void SC18IS602::configure(uint8_t cfg)
{
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(FUN_CONFIG);
  i2cBus->addWriteData(cfg, I2Cbus::RWF_stop);
  i2cBus->execute();
}

/*
        Write data to bridge buffer
*/
void SC18IS602::spiWrite(uint8_t slave, int len, uint8_t *data)
{
  if (slave > 3) slave = 3;

  if (len == 0) return;

  if (len > BUFFER_LEN) len = BUFFER_LEN;

  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Write);
  i2cBus->addWriteData(FUN_SPI | (1 << slave));

  while (len--)
    i2cBus->addWriteData(*data++, (len == 0) ? I2Cbus::RWF_stop : 0);

  // send commands packet
  i2cBus->execute();
}

/*
        Read data from bridge buffer
*/
void SC18IS602::spiReadBuffer(int len, uint8_t *data)
{
  uint32_t  readBuffer[BUFFER_LEN];
  uint32_t *rb = readBuffer;
  int       l;

  if (len == 0) return;

  if (len > BUFFER_LEN) len = BUFFER_LEN;

  // Read data
  i2cBus->addAddress(i2c_deviceAddress, I2Cbus::I2C_Read);
  l = len;
  while (l--)
    i2cBus->addRead(rb++, (l == 0) ? (I2Cbus::RWF_stop | I2Cbus::RWF_dontAck) : 0);

  // send commands packet
  i2cBus->execute();

  // copy data
  l  = len;
  rb = readBuffer;
  while (l--)
    *data++ = *rb++;
}

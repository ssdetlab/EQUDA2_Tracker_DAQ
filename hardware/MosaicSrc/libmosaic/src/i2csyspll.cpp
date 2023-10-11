/*
 * Copyright (C) 2014
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2014.
 *
 */
#include "i2csyspll.h"
#include "mexception.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define CDCM6208_ADDRESS 0x54

I2CSysPll::I2CSysPll(WishboneBus *wbbPtr, uint32_t baseAdd) : I2Cbus(wbbPtr, baseAdd) {}

void I2CSysPll::writeReg(uint8_t add, uint16_t d)
{
  addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Write);
  addWriteData(0x00);
  addWriteData(add);
  addWriteData(d >> 8);
  addWriteData(d & 0xff, I2Cbus::RWF_stop);
  execute();
}

void I2CSysPll::readReg(uint8_t add, uint16_t *d)
{
  uint32_t *r = new uint32_t[2];

  addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Write);
  addWriteData(0x00);
  addWriteData(add, I2Cbus::RWF_stop);

  addAddress(CDCM6208_ADDRESS, I2Cbus::I2C_Read);
  addRead(r);
  addRead(r + 1, I2Cbus::RWF_dontAck | I2Cbus::RWF_stop);
  execute();

  *d = ((r[0] & 0xff) << 8) | (r[1] & 0xff);
}

void I2CSysPll::setup(pllRegisters_t regs)
{
  uint16_t r;
  //	int lookTry;

  // Write
  for (int i = 0; i < 20; i++) {
    writeReg(i, regs.reg[i]);
  }

  // Verify
  for (int i = 0; i < 20; i++) {
    readReg(i, &r);
    if (r != regs.reg[i]) throw MBoardInitError("System PLL verify error");
  }

  // Cycle the reset
  writeReg(3, regs.reg[3] & ~(1 << 6));
  writeReg(3, regs.reg[3]);

#if 0 // lock check is done at upper level
      // wait for PLL to lock
	lookTry = 500;
	while (--lookTry){
		readReg(21, &r);
		if ((r&0x0004) == 0)
			break;
	}

	if (lookTry==0)
			throw MBoardInitError("System PLL NOT locked!");
#endif
}

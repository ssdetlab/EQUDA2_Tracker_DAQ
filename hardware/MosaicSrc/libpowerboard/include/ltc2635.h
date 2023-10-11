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

#ifndef LTC2635_H
#define LTC2635_H

#include "i2cslave.h"
#include <stdint.h>

class LTC2635 : public I2Cslave {
public:
  LTC2635(I2Cbus *bus, uint8_t address);
  void write(uint8_t cmd, uint8_t add = 0, uint16_t data = 0);

private:
  enum {
    CMD_WriteReg          = 0x00,
    CMD_UpdateReg         = 0x10,
    CMD_WriteRegUpdateAll = 0x20,
    CMD_WriteUpdateReg    = 0x30,
    CMD_PowerDownN        = 0x40,
    CMD_PowerDownChip     = 0x50,
    CMD_SelIntRef         = 0x60,
    CMD_SelExtRef         = 0x70
  };

  // uint8_t i2c_baseAddress;

public:
  void WriteReg(uint8_t add, uint16_t data) { write(CMD_WriteReg, add, data); }

  void UpdateReg(uint8_t add) { write(CMD_UpdateReg, add); }

  void WriteRegUpdateAll(uint8_t add, uint16_t data) { write(CMD_WriteRegUpdateAll, add, data); }

  void WriteUpdateReg(uint8_t add, uint16_t data) { write(CMD_WriteUpdateReg, add, data); }

  void PowerDownN(uint8_t add) { write(CMD_PowerDownN, add); }

  void PowerDownChip() { write(CMD_PowerDownChip); }

  void SelIntRef() { write(CMD_SelIntRef); }

  void SelExtRef() { write(CMD_SelExtRef); }
};

#endif // LTC2635_H

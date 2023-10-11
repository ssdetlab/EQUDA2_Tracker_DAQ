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

#ifndef AD5254_H
#define AD5254_H

#include "i2cslave.h"
#include <stdint.h>

class AD5254 : public I2Cslave {
public:
  AD5254(I2Cbus *bus, uint8_t address);
  void    write(uint8_t cmd, uint8_t ch, uint8_t data);
  void    write(uint8_t cmd, uint8_t ch);
  uint8_t read(uint8_t cmd, uint8_t add);
  void    ackPolling();

private:
  enum {
    CMD_SetRDAC     = 0x00,
    CMD_WriteEEMEM  = 0x20, // Write (store) RDAC Setting and User-defined Data to EEMEM
    CMD_ReadEEMEM   = 0x20, // Read (restore) RDAC Setting and User-defined Data from EEMEM
    CMD_NOP         = 0x00, // NOP command
    CMD_RestoreAll  = 0xb8, // Restore EEMEMs to all RDACs
    CMD_RestoreRDAC = 0x88, // Restore single EEMEM to RDAC
    CMD_StoreRDAC   = 0x90, // Store single RDAC to EEMEM
  };

public:
  void restoreAll()
  {
    write(CMD_RestoreAll, 0);
    ackPolling();
  }

  void setRDAC(uint8_t ch, uint8_t data) { write(CMD_SetRDAC, ch, data); }

  void storeRDAC(uint8_t ch)
  {
    write(CMD_StoreRDAC, ch);
    ackPolling();
  }

  void restoreRDAC(uint8_t ch)
  {
    write(CMD_RestoreRDAC, ch);
    write(CMD_NOP, 0);
  }

  uint8_t getRDAC(uint8_t ch) { return read(CMD_SetRDAC, ch); }

  uint8_t getEEMEM(uint8_t add) { return read(CMD_WriteEEMEM, add); }
};

#endif // AD5254_H

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

#ifndef MAX5419_H
#define MAX5419_H

#include "i2cslave.h"
#include <stdint.h>

class MAX5419 : public I2Cslave {
public:
  MAX5419(I2Cbus *bus, uint8_t address);
  void setRDAC(uint8_t b);
  void setNVREG(uint8_t b);
  void restoreRDAC();
  void storeRDAC();

private:
  enum { CMD_VREG = 0x11, CMD_NVREG = 0x21, CMD_NVREGxVREG = 0x61, CMD_VREGxNVREG = 0x51 };
};

#endif // MAX5419_H

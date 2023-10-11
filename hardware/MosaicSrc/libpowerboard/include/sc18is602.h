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

#ifndef SC18IS602_H
#define SC18IS602_H

#include "i2cslave.h"
#include <stdint.h>

class SC18IS602 : public I2Cslave {
public:
  SC18IS602(I2Cbus *bus, uint8_t address);
  void configure(uint8_t cfg);
  void spiWrite(uint8_t slave, int len, uint8_t *data);
  void spiReadBuffer(int len, uint8_t *data);

private:
  // bridge functions
  enum {
    FUN_SPI       = 0x00, // bits 3-0 are the SPI target SSn pins
    FUN_CONFIG    = 0xf0, // bridge configuration
    FUN_CLEAR_INT = 0xf1, // Clear the interrupt
    FUN_IDLE      = 0xf2, // Set Idle mode (Low power)
    FUN_GPIO_WR   = 0xf4, // Write the GPIO port
    FUN_GPIO_RD   = 0xf5, // Read the GPIO port
    FUN_GPIO_EN   = 0xf6,
    FUN_GPIO_CFG  = 0xf7
  };

  // bits for function "config"
  enum {
    CFG_RATE_1843 = 0x00, // SPI clock at 1843 KHz
    CFG_RATE_461  = 0x01, // SPI clock at 461 KHz
    CFG_RATE_115  = 0x02, // SPI clock at 115 KHz
    CFG_RATE_58   = 0x03, // SPI clock at 58 KHz
    CFG_CPHA      = 0x04,
    CFG_CPOL      = 0x08,
    CFG_ORDER_LSB = 0x20 // LSB transmitted first
  };
};

#endif // SC18IS602_H

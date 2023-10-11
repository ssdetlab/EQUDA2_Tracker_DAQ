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

#ifndef AD7997_H
#define AD7997_H

#include "i2cslave.h"
#include <stdint.h>

class AD7997 : public I2Cslave {
public:
  AD7997(I2Cbus *bus, uint8_t address);
  void write(uint8_t address, uint8_t data);
  void write(uint8_t address, uint16_t data);
  void addWriteAddressPointer(uint8_t address);
  void read(uint8_t address, uint8_t *data);
  void read(uint8_t address, uint16_t *data);
  void setConfiguration(uint8_t chMap);
  void convert(int nch, uint16_t *result);

private:
  enum {
    CMD_NOP      = 0x00,
    CMD_CONV1    = 0x80, // Convert Vin1
    CMD_CONV2    = 0x90, // Convert Vin2
    CMD_CONV3    = 0xa0, // Convert Vin3
    CMD_CONV4    = 0xb0, // Convert Vin4
    CMD_CONV5    = 0xc0, // Convert Vin5
    CMD_CONV6    = 0xd0, // Convert Vin6
    CMD_CONV7    = 0xe0, // Convert Vin7
    CMD_CONV8    = 0xf0, // Convert Vin8
    CMD_CONV_SEQ = 0x70  // Convert sequence of configured channels
  };

public:
  enum {
    REG_ConvertionReult = 0x00,
    REG_AllertStatus    = 0x01,
    REG_Configuration   = 0x02,
    REG_CycleTimer      = 0x03,
    REG_DataL_CH1       = 0x04,
    REG_DataH_CH1       = 0x05,
    REG_Hysteresis_CH1  = 0x06,
    REG_DataL_CH2       = 0x07,
    REG_DataH_CH2       = 0x08,
    REG_Hysteresis_CH2  = 0x09,
    REG_DataL_CH3       = 0x0a,
    REG_DataH_CH3       = 0x0b,
    REG_Hysteresis_CH3  = 0x0c,
    REG_DataL_CH4       = 0x0d,
    REG_DataH_CH4       = 0x0e,
    REG_Hysteresis_CH4  = 0x0f
  };

  enum {
    CFG_BusyAllertPolarity = 1 << 0,
    CFG_BusyAllert         = 1 << 1,
    CFG_AllertEn           = 1 << 2,
    CFG_FLTR               = 1 << 3
  };
};

#endif // AD7997_H

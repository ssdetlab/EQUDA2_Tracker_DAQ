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

#ifndef ADC128D818_H
#define ADC128D818_H

#include "i2cslave.h"
#include <stdint.h>

class ADC128D818 : public I2Cslave {
public:
  ADC128D818(I2Cbus *bus, uint8_t address);

  void write(uint8_t address, uint8_t data);
  void addWriteAddressPointer(uint8_t address);
  void read(uint8_t address, uint8_t *data);
  void read(uint8_t address, uint16_t *data);
  void setConfiguration();
  void getConfiguration(uint8_t *data);
  void convert(uint16_t *result);

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
    REG_Configuration         = 0x00,
    REG_InterruptStatus       = 0x01,
    REG_IterruptMask          = 0x03,
    REG_ConvertionRate        = 0x07,
    REG_ChanneDisable         = 0x08,
    REG_OneShot               = 0x09,
    REG_DeepShutdown          = 0x0a,
    REG_AdvancedConfiguration = 0x0b,
    REG_BusyStatus            = 0x0c,
    REG_ChannelReadings       = 0x20, // from 0x20 to 0x27	(16 bits)
    REG_Limit                 = 0x2a, // from 0x2a to 0x39
    REG_ManufacturerID        = 0x3e,
    REG_RevisionID            = 0x3f
  };

  enum {
    ADV_CFG_ExtRefEnable = 0x01 << 0,
    ADV_CFG_Mode0        = 0x00 << 1,
    ADV_CFG_Mode1        = 0x01 << 1,
    ADV_CFG_Mode2        = 0x02 << 1,
    ADV_CFG_Mode3        = 0x03 << 1,
    CFG_Start            = 1 << 0,
    CFG_INTnn_En         = 1 << 1,
    CFG_INTnn_Clr        = 1 << 3,
    CFG_INIT             = 1 << 7
  };
};

#endif // ADC128D818_H

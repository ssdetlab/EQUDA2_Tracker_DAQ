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

#ifndef CONTROLINTERFACE_H
#define CONTROLINTERFACE_H

#include "mwbbslave.h"
#include <mutex>
#include <stdint.h>

class CiReadRequest {
public:
  uint8_t   chipID;        // requested Chip
  uint16_t  address;       // register address
  uint32_t  IPBusReadData; // space where put IPBus read
  uint16_t *readDataPtr;   // pointer to the final data place
};

class ControlInterface : public MWbbSlave {
public:
  ControlInterface();
  ControlInterface(WishboneBus *wbbPtr, uint32_t baseAddress);
  ~ControlInterface();
  void setBusAddress(WishboneBus *wbbPtr, uint32_t baseAddress);
  void setPhase(uint8_t phase);
  void addEnable(bool en);
  void addDisableME(bool dis);
  void addGetErrorCounter(uint32_t *ctr);
  void addSendCmd(uint8_t cmd);
  void addWriteReg(uint8_t chipID, uint16_t address, uint16_t data);
  void addReadReg(uint8_t chipID, uint16_t address, uint16_t *dataPtr);
  void execute();

  enum opCode_e {
    OPCODE_STROBE_2  = 0xb1, // Trigger
    OPCODE_GRST      = 0xd2, // Global chip reset
    OPCODE_RORST     = 0x63, // Readout reset
    OPCODE_PRST      = 0xe4, // Pixel matrix reset
    OPCODE_STROBE_6  = 0x55, // Trigger
    OPCODE_BCRST     = 0x36, // Bunch Counter Reset
    OPCODE_ECRST     = 0x87, // Event Counter Reset
    OPCODE_PULSE     = 0x78, // Calibration Pulse trigger
    OPCODE_STROBE_10 = 0xc9, // Trigger
    OPCODE_RSVD1     = 0xaa, // Reserved for future use
    OPCODE_RSVD2     = 0x1b, // Reserved for future use
    OPCODE_WROP      = 0x9c, // Start Unicast or Multicast Write
    OPCODE_STROBE_14 = 0x2d, // Trigger
    OPCODE_RDOP      = 0x4e  // Start Unicast Read
  };

private: // WBB Slave registers map
  enum regAddress_e { regWriteCtrl = 0, regWriteData = 1, regReadData = 2, regConfig = 3 };

  enum readFlagsBits_e {
    FLAG_SYNC_BIT   = (1 << 3),
    FLAG_CHIPID_BIT = (1 << 2),
    FLAG_DATAL_BIT  = (1 << 1),
    FLAG_DATAH_BIT  = (1 << 0)
  };

  enum configBits_e { CFG_PHASE_MASK = 0x03, CFG_EN = (1 << 3), CFG_DISABLE_ME = (1 << 4) };

private:
  CiReadRequest *      readReqest;
  int                  readRequestSize;
  int                  numReadRequest;
  std::recursive_mutex mutex;
};

#endif // CONTROLINTERFACE_H

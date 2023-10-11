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

#ifndef ALPIDERCV_H
#define ALPIDERCV_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>


class ALPIDErcv : public MWbbSlave {
public:
  ALPIDErcv();
  ALPIDErcv(WishboneBus *wbbPtr, uint32_t baseAddress);
  ~ALPIDErcv();
  void setBusAddress(WishboneBus *wbbPtr, uint32_t baseAddress);
  void addSetReg(uint16_t address, uint16_t val);
  void addGetReg(uint16_t address, uint32_t *val);
  void addEnable(bool d);
  void addInvertInput(bool d);
  void reset();

  // Access to transceiver RDP registers
  void addSetRDPReg(uint16_t address, uint16_t val);
  void addGetRDPReg(uint16_t address, uint32_t *val);
  void addSetRDPRegField(uint16_t address, uint16_t size, uint16_t offset, uint16_t val);

  // RX pattern check related functions
  void addPRBSsetSel(uint8_t s);
  void addPRBSreset();
  void addGetPRBScounter(uint32_t *ctr);

  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e { regOpMode = 0, regPrbs = 1, regReset = 2, rdpBase = 0x00800000 };

  enum opModeBits_e { OPMODE_RCVENABLE = (1 << 0), OPMODE_INVERT_POLARITY = (1 << 1) };

  enum prbsBits_e { PRBS_RESET = (1 << 3), PRBS_SEL_MASK = 0x07, PRBS_SEL_SHIFT = 0 };

  enum resetBits_e { RESET_GTP_DONE = (1 << 1), RESET_ALIGNED = (1 << 0) };

public:
  enum prbsSel_e { PRBS_NONE = 0, PRBS_7 = 1, PRBS_15 = 2, PRBS_23 = 3, PRBS_31 = 4 };
};
#endif // ALPIDERCV_H

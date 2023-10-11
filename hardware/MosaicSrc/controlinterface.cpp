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
 * 21/12/2015	Added mutex for multithread operation
 */
#include "controlinterface.h"
#include "pexception.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

ControlInterface::ControlInterface()
{
  readReqest     = NULL;
  numReadRequest = 0;
}

ControlInterface::ControlInterface(WishboneBus *wbbPtr, uint32_t baseAdd)
    : MWbbSlave(wbbPtr, baseAdd)
{
  readRequestSize = wbb->getBufferSize() / (5 * 4); // every read on IPBus requires 5 word
  readReqest      = new CiReadRequest[readRequestSize];
  numReadRequest  = 0;
}

void ControlInterface::setBusAddress(WishboneBus *wbbPtr, uint32_t baseAdd)
{
  // set the WBB
  MWbbSlave::setBusAddress(wbbPtr, baseAdd);

  readRequestSize = wbbPtr->getBufferSize() / (5 * 4); // every read on IPBus requires 5 word
  if (readReqest) delete readReqest;
  readReqest     = new CiReadRequest[readRequestSize];
  numReadRequest = 0;
}

ControlInterface::~ControlInterface()
{
  if (readReqest) delete readReqest;
}

//
//	Control the output of FE clock to ALPIDE chip
//
void ControlInterface::addEnable(bool en)
{
  wbb->addRMWbits(baseAddress + regConfig, ~CFG_EN, en ? CFG_EN : 0);
}

//
//	Control the Manchester encoding
//
void ControlInterface::addDisableME(bool dis)
{
  wbb->addRMWbits(baseAddress + regConfig, ~CFG_DISABLE_ME, dis ? CFG_DISABLE_ME : 0);
}

//
//	set the output phase
//
void ControlInterface::setPhase(uint8_t phase)
{
  wbb->addRMWbits(baseAddress + regConfig, ~CFG_PHASE_MASK, phase);
  wbb->execute();
}

//
//	Read error counter (Works only on dedicated firmware)
//
void ControlInterface::addGetErrorCounter(uint32_t *ctr)
{
  wbb->addRead(baseAddress + regConfig, ctr);
}

//
// schedule a broadcast command
//
void ControlInterface::addSendCmd(uint8_t cmd)
{
  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addWrite(baseAddress + regWriteCtrl, cmd << 24);
}

//
// schedule a register write
//
void ControlInterface::addWriteReg(uint8_t chipID, uint16_t address, uint16_t data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  wbb->addWrite(baseAddress + regWriteData, data);
  wbb->addWrite(baseAddress + regWriteCtrl,
                (OPCODE_WROP << 24) | ((chipID & 0xff) << 16) | (address & 0xffff));
}

//
// schedule a register read
//
void ControlInterface::addReadReg(uint8_t chipID, uint16_t address, uint16_t *dataPtr)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  if (!wbb) throw PControlInterfaceError("No IPBus configured");

  if (numReadRequest >= readRequestSize) execute();

  // queue read command
  wbb->addWrite(baseAddress + regWriteCtrl,
                (OPCODE_RDOP << 24) | ((chipID & 0xff) << 16) | (address & 0xffff));
  // read answer
  wbb->addRead(baseAddress + regReadData, &readReqest[numReadRequest].IPBusReadData);

  // Put the request into the list
  readReqest[numReadRequest].chipID        = chipID;
  readReqest[numReadRequest].address       = address;
  readReqest[numReadRequest].IPBusReadData = 0;
  readReqest[numReadRequest].readDataPtr   = dataPtr;
  numReadRequest++;
}

void ControlInterface::execute()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  std::string errMsg = "";

  try {
    MWbbSlave::execute();

    // check the read results
    for (int i = 0; i < numReadRequest; i++) {
      uint32_t          d        = readReqest[i].IPBusReadData;
      uint8_t           rxChipID = (d >> 16) & 0xff;
      uint8_t           rxFlags  = (d >> 24) & 0x0f;
      std::stringstream ss;
      ss << std::hex << readReqest[i].address;

      // check the flags
      if ((rxFlags & FLAG_SYNC_BIT) == 0) {
        errMsg = "Sync error reading data, rxChipID: " + std::to_string(static_cast<int>(rxChipID));
        errMsg += ". txChipID: " + std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". Address: 0x" + ss.str();
        throw PControlInterfaceError(errMsg);
      }
      if ((rxFlags & FLAG_CHIPID_BIT) == 0) {
        errMsg = "No ChipID reading data, rxChipID: " + std::to_string(static_cast<int>(rxChipID));
        errMsg += ". txChipID: " + std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". Address: 0x" + ss.str();
        throw PControlInterfaceError(errMsg);
      }
      if ((rxFlags & FLAG_DATAL_BIT) == 0) {
        errMsg = "No Data Low byte reading data, rxChipID: " +
                 std::to_string(static_cast<int>(rxChipID));
        errMsg += ". txChipID: " + std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". Address: 0x" + ss.str();
        throw PControlInterfaceError(errMsg);
      }
      if ((rxFlags & FLAG_DATAH_BIT) == 0) {
        errMsg = "No Data High byte reading data, rxChipID: " +
                 std::to_string(static_cast<int>(rxChipID));
        errMsg += ". txChipID: " + std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". Address: 0x" + ss.str();
        throw PControlInterfaceError(errMsg);
      }
      // check the sender
      if (rxChipID != readReqest[i].chipID) {
        errMsg = "ChipID mismatch, rxChipID: " + std::to_string(static_cast<int>(rxChipID)) +
                 ", is not equal to replied chip ID: " +
                 std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". txChipID: " + std::to_string(static_cast<int>(readReqest[i].chipID));
        errMsg += ". Address: 0x" + ss.str();
        throw PControlInterfaceError(errMsg);
      }

      *readReqest[i].readDataPtr = (d & 0xffff);
    }
    numReadRequest = 0;
  }
  catch (...) {
    numReadRequest = 0;
    throw;
  }
}

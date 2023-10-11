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

#ifndef IPBUS_H
#define IPBUS_H

#include "wishbonebus.h"
#include <mutex>
#include <stdint.h>
#include <string>

#define IPBUS_PROTOCOL_VERSION 2
#define WRONG_PROTOCOL_VERSION 3
#define DEFAULT_PACKET_SIZE 1400

class IPbusTransaction {
public:
  uint8_t   version;
  uint16_t  words;
  uint8_t   typeId;
  uint8_t   transactionId;
  uint8_t   infoCode;
  uint32_t *readDataPtr;
};

class IPbus : public WishboneBus {
public:
  IPbus(int pktSize = DEFAULT_PACKET_SIZE);
  ~IPbus();
  void         addIdle();
  void         addWrite(uint32_t address, uint32_t data);
  void         addWrite(int size, uint32_t address, uint32_t *data);
  void         addNIWrite(int size, uint32_t address, uint32_t *data);
  void         addRead(int size, uint32_t address, uint32_t *data);
  void         addRead(uint32_t address, uint32_t *data) { addRead(1, address, data); }
  void         addNIRead(int size, uint32_t address, uint32_t *data);
  void         addRMWbits(uint32_t address, uint32_t mask, uint32_t data, uint32_t *rData = NULL);
  void         addRMWsum(uint32_t address, uint32_t data, uint32_t *rData = NULL);
  virtual void execute() = 0;
  int          getBufferSize() { return bufferSize; }
  virtual const std::string name() = 0;

  // test functions
  void addBadIdle(bool sendWrongVersion = false, bool sendWrongInfoCode = false);
  void setBufferSize(int s) { bufferSize = s; }
  void cutTX(int size) { txSize -= size; }

private:
  void     chkBuffers(int txTransactionSize, int rxTransactionSize);
  void     addWord(uint32_t w);
  uint32_t getWord();
  void     addHeader(uint16_t words, uint8_t typeId, uint32_t *readDataPtr);
  void     getHeader(IPbusTransaction *tr);
  void     clearList();
  void     dumpRxData();

public:
  // IPBus info codes (errors)
  enum infoCode_e {
    infoCodeSuccess         = 0x0,
    infoCodeBadHeader       = 0x1,
    infoCodeBusErrRead      = 0x2,
    infoCodeBusErrWrite     = 0x3,
    infoCodeBusTimeoutRead  = 0x4,
    infoCodeBusTimeoutWrite = 0x5,
    infoCodeBufferOverflaw  = 0x6,
    infoCodeBufferUnderflaw = 0x7,
    infoCodeRequest         = 0xf
  };

  // IPBus type of transaction
  enum typeId_e {
    typeIdRead    = 0x0,
    typeIdWrite   = 0x1,
    typeIdNIRead  = 0x2,
    typeIdNIWrite = 0x3,
    typeIdRMWbits = 0x4,
    typeIdRMWsum  = 0x5,
    typeIdIdle    = 0xf
  };

protected:
  uint8_t *            txBuffer;
  uint8_t *            rxBuffer;
  int                  txSize;
  int                  rxSize;
  int                  errorCode;
  std::recursive_mutex mutex;

private:
  int                     bufferSize;
  class IPbusTransaction *transactionList;
  int                     numTransactions;
  uint8_t                 transactionId;
  int                     expectedRxSize;
  int                     rxPtr;
  int                     lastRxPktId;

protected:
  bool duplicatedRxPkt();
  void processAnswer();
  int  getExpectedRxSize() { return expectedRxSize; }
};

#endif // IPBUS_H

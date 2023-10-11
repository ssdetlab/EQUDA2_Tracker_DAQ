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
#include "ipbus.h"
#include "ipbusudp.h"
#include "mexception.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// #define TRACE_IPBUS

#ifdef TRACE_IPBUS
#define TRACE(format, ...)                                                                         \
  {                                                                                                \
    fprintf(stderr, "%s ", name().c_str());                                                        \
    fprintf(stderr, format, ##__VA_ARGS__);                                                        \
    fflush(stderr);                                                                                \
  }
#else
#define TRACE(format, ...)
#endif

IPbus::IPbus(int pktSize)
{
  bufferSize      = pktSize;
  txBuffer        = new uint8_t[bufferSize];
  rxBuffer        = new uint8_t[bufferSize];
  transactionList = new IPbusTransaction[bufferSize / 4];
  transactionId   = 0;
  lastRxPktId     = 0;
  clearList();
}

IPbus::~IPbus()
{
  delete transactionList;
  delete txBuffer;
  delete rxBuffer;
}

void IPbus::clearList()
{
  numTransactions = 0;
  txSize          = 0;
  expectedRxSize  = 0;
  rxPtr           = 0;
}

void IPbus::addWord(uint32_t w)
{
  txBuffer[txSize++] = (w >> 24) & 0xff;
  txBuffer[txSize++] = (w >> 16) & 0xff;
  txBuffer[txSize++] = (w >> 8) & 0xff;
  txBuffer[txSize++] = w & 0xff;
}

uint32_t IPbus::getWord()
{
  uint32_t w;

  w = (rxBuffer[rxPtr++] & 0xff) << 24;
  w |= (rxBuffer[rxPtr++] & 0xff) << 16;
  w |= (rxBuffer[rxPtr++] & 0xff) << 8;
  w |= (rxBuffer[rxPtr++] & 0xff);

  return w;
}

void IPbus::addHeader(uint16_t words, uint8_t typeId, uint32_t *readDataPtr)
{
  // Avoid consecutive packets with the same transactionId in first IPBUS request
  if ((numTransactions == 0) && (transactionId == lastRxPktId)) {
    transactionId++;
    TRACE("IPbus::addHeader increased transactionId to %d\n", transactionId);
  }

  // Put the request into the list
  transactionList[numTransactions].words         = words;
  transactionList[numTransactions].transactionId = transactionId;
  transactionList[numTransactions].typeId        = typeId;
  transactionList[numTransactions].readDataPtr   = readDataPtr;

  // Add the header to the tx buffer
  addWord((IPBUS_PROTOCOL_VERSION << 28) | (words << 16) | (transactionId << 8) | (typeId << 4) |
          infoCodeRequest);

  transactionId++;
  transactionId &= 0xff;
  numTransactions++;
}

void IPbus::getHeader(IPbusTransaction *tr)
{
  uint32_t header = getWord();

  tr->version       = (header >> 28) & 0x0f;
  tr->words         = (header >> 16) & 0xfff;
  tr->transactionId = (header >> 8) & 0xff;
  tr->typeId        = (header >> 4) & 0xf;
  tr->infoCode      = header & 0xf;
  ;

#ifdef TRACE_IPBUS
//	printf("header: %08x\n", header);
#endif
}

void IPbus::chkBuffers(int txTransactionSize, int rxTransactionSize)
{
  if ((bufferSize - txSize) < txTransactionSize ||
      (bufferSize - expectedRxSize) < rxTransactionSize) {
#ifdef TRACE_IPBUS
    cout << "flush the command buffer" << endl;
#endif
    execute();
  }

  if (txTransactionSize > bufferSize) throw MIPBusError("Tx buffer overflaw");

  if (rxTransactionSize > bufferSize) throw MIPBusError("Rx buffer overflaw");
}

void IPbus::addIdle()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  TRACE("IPbus::addIdle%s\n", "");
  chkBuffers(4, 4);
  addHeader(0, typeIdIdle, NULL);
  expectedRxSize += 4;
}

void IPbus::addWrite(int size, uint32_t address, uint32_t *data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  TRACE("IPbus::addWrite (size:%d, address:0x%08x, *data:0x%08lx)\n", size, address,
        (unsigned long)data);
  chkBuffers(4 * (size + 2), 4);
  addHeader(size, typeIdWrite, NULL);
  addWord(address);
  for (int i = 0; i < size; i++)
    addWord(*data++);
  expectedRxSize += 4;
}

void IPbus::addNIWrite(int size, uint32_t address, uint32_t *data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  TRACE("IPbus::addNIWrite (size:%d, address:0x%08x, *data:0x%08lx)\n", size, address,
        (unsigned long)data);
  chkBuffers(4 * (size + 2), 4);
  addHeader(size, typeIdNIWrite, NULL);
  addWord(address);
  for (int i = 0; i < size; i++)
    addWord(*data++);
  expectedRxSize += 4;
}

void IPbus::addWrite(uint32_t address, uint32_t data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  TRACE("IPbus::addWrite (address:0x%08x, data:0x%08lx)\n", address, (unsigned long)data);
  chkBuffers(4 * (1 + 2), 4);
  addHeader(1, typeIdWrite, NULL);
  addWord(address);
  addWord(data);
  expectedRxSize += 4;
}

void IPbus::addRead(int size, uint32_t address, uint32_t *data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  TRACE("IPbus::addRead (size:%d, address:0x%08x, *data:0x%08lx)\n", size, address,
        (unsigned long)data);
  chkBuffers(4 * 2, 4 * (size + 1));
  addHeader(size, typeIdRead, data);
  addWord(address);
  expectedRxSize += 4 + size * 4;
}

void IPbus::addNIRead(int size, uint32_t address, uint32_t *data)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  TRACE("IPbus::addMIRead (size:%d, address:0x%08x, *data:0x%08lx)\n", size, address,
        (unsigned long)data);
  chkBuffers(4 * 2, 4 * (size + 1));
  addHeader(size, typeIdNIRead, data);
  addWord(address);
  expectedRxSize += 4 + size * 4;
}

void IPbus::addRMWbits(uint32_t address, uint32_t mask, uint32_t data, uint32_t *rData)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  TRACE("IPbus::addRMWbits (address:0x%08x, mask:0x%08lx, data:0x%08lx)\n", address,
        (unsigned long)mask, (unsigned long)data);
  chkBuffers(4 * (1 + 3), 4 * 2);
  addHeader(1, typeIdRMWbits, rData);
  addWord(address);
  addWord(mask);
  addWord(data);
  expectedRxSize += 8;
}

void IPbus::addRMWsum(uint32_t address, uint32_t data, uint32_t *rData)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  TRACE("IPbus::addRMWsum (address:0x%08x, data:0x%08lx)\n", address, (unsigned long)data);
  chkBuffers(4 * (1 + 2), 4 * 2);
  addHeader(1, typeIdRMWsum, rData);
  addWord(address);
  addWord(data);
  expectedRxSize += 8;
}

bool IPbus::duplicatedRxPkt()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  IPbusTransaction                      tr;

  rxPtr = 0;
  getHeader(&tr);

  //	TRACE("IPbus::duplicatedRxPkt transactionId:%d lastRxPktId:%d\n", tr.transactionId,
  // lastRxPktId);

  if (tr.transactionId == lastRxPktId) return true;

  return false;
}

void IPbus::processAnswer()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  IPbusTransaction                      tr;
  int                                   pktId = 0;

  try {
    rxPtr = 0;
#ifdef TRACE_IPBUS
    dumpRxData();
#endif
    for (int i = 0; i < numTransactions; i++) {

      IPbusUDP *udpbus  = dynamic_cast<IPbusUDP *>(this);
      string    address = "";

      if (udpbus) address = udpbus->getIPaddress();
      if ((rxSize - rxPtr) < 4) {
        // printf("\n\n Wrong answer size at transaction %d. size:%d\n\n", i, rxSize-rxPtr);
        throw MIPBusError("Wrong answer size", address);
      }

      getHeader(&tr);

      // check the header
      if (tr.version != IPBUS_PROTOCOL_VERSION)
        throw MIPBusError("Wrong version in answer", address);

      if (tr.transactionId != transactionList[i].transactionId)
        throw MIPBusError("Wrong transaction ID in answer", address);
      if (i == 0) pktId = tr.transactionId;

      if (tr.typeId != transactionList[i].typeId)
        throw MIPBusError("Wrong transaction type in answer", address);

      if (tr.infoCode != infoCodeSuccess) {
        switch (tr.infoCode) {
        case infoCodeBadHeader:
          throw MIPBusError("Remote bus error BAD HEADER", address);
        case infoCodeBusErrRead:
          throw MIPBusError("Remote bus error in read", address);
        case infoCodeBusErrWrite:
          throw MIPBusErrorWrite("Remote bus error in write", address);
        case infoCodeBusTimeoutRead:
          throw MIPBusErrorReadTimeout("Remote bus timeout in read", address);
        case infoCodeBusTimeoutWrite:
          throw MIPBusError("Remote bus timeout in write", address);
        case infoCodeBufferOverflaw:
          throw MIPBusError("Remote bus overflow TX buffer error", address);
        case infoCodeBufferUnderflaw:
          throw MIPBusError("Remote bus underflow RX buffer error", address);
        }
      }

      if (tr.words != transactionList[i].words)
        throw MIPBusError("Wrong number of words in transaction answer", address);

      // get data
      if (tr.typeId == typeIdRead || tr.typeId == typeIdNIRead || tr.typeId == typeIdRMWbits ||
          tr.typeId == typeIdRMWsum) {

        if ((rxSize - rxPtr) < (tr.words * 4)) throw MIPBusError("Wrong answer size", address);

        if (transactionList[i].readDataPtr != NULL)
          for (int j = 0; j < tr.words; j++)
            transactionList[i].readDataPtr[j] = getWord();
        else
          for (int j = 0; j < tr.words; j++)
            getWord();
      }
    }
    clearList();
    lastRxPktId = pktId; // store the last packet ID
  }
  catch (...) {
    clearList();
    throw;
  }
}

/*
 *		Debug function tu dump received data
 */
void IPbus::dumpRxData()
{
  int rxPtrSave = rxPtr;

  printf("Received IPBUS data:\n");
  for (int i = 0; i < rxSize; i += 4)
    printf("%08x\n", getWord());

  rxPtr = rxPtrSave;
}

/*
 *		Test functions
 */
void IPbus::addBadIdle(bool sendWrongVersion, bool sendWrongInfoCode)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  TRACE("IPbus::addBadIdle%s\n", "");
  chkBuffers(4, 4);

  // modified copy of addHeader function
  // void IPbus::addHeader(uint16_t words, uint8_t typeId, uint32_t *readDataPtr)
  //	addHeader(0, typeIdIdle, NULL);
  uint16_t  words       = 0;
  uint8_t   typeId      = typeIdIdle;
  uint32_t *readDataPtr = NULL;

  // Avoid consecutive packets with the same transactionId in first IPBUS request
  if ((numTransactions == 0) && (transactionId == lastRxPktId)) transactionId++;

  // Put the request into the list
  transactionList[numTransactions].words         = words;
  transactionList[numTransactions].transactionId = transactionId;
  transactionList[numTransactions].typeId        = typeId;
  transactionList[numTransactions].readDataPtr   = readDataPtr;

  // Add the header to the tx buffer
  unsigned ipProtocol = sendWrongVersion ? WRONG_PROTOCOL_VERSION : IPBUS_PROTOCOL_VERSION;
  unsigned infoCode   = sendWrongInfoCode ? infoCodeBusErrRead : infoCodeRequest;
  addWord((ipProtocol << 28) | (words << 16) | (transactionId << 8) | (typeId << 4) | infoCode);

  transactionId++;
  transactionId &= 0xff;
  numTransactions++;

  expectedRxSize += 4;
}

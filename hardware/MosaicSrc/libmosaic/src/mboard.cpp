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
#include "mboard.h"
#include "mdatareceiver.h"
#include "mexception.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#define PLATFORM_IS_LITTLE_ENDIAN

// #define DEBUG_HEADERS

void MBoard::init()
{
  mIPbus = new IPbusUDP();

  // Data Generator
  mDataGenerator = new MDataGenerator(mIPbus, WbbBaseAddress::dataGenerator);

  // Run control
  mRunControl = new MRunControl(mIPbus, WbbBaseAddress::runControl);

  // Trigger control
  mTriggerControl = new MTriggerControl(mIPbus, WbbBaseAddress::triggerControl);

  // System PLL on I2C bus
  mSysPLL = new I2CSysPll(mIPbus, WbbBaseAddress::i2cSysPLL);

  tcp_sockfd       = -1;
  numReceivers     = 0;
  timer_fd         = -1;
  TCPtimeout       = -1;
  ignoreTimeouts   = true;
  insideDataPacket = false;
}

MBoard::MBoard() { init(); }

MBoard::MBoard(const char *IPaddr, int port)
{
  init();
  setIPaddress(IPaddr, port);
}

void MBoard::setIPaddress(const char *IPaddr, int port)
{
  IPaddress = IPaddr;
  mIPbus->setIPaddress(IPaddr, port);
}

MBoard::~MBoard()
{
  // avoid flushDataReceivers
  numReceivers = 0;

  // close the TCP connection
  closeTCP();

  // delete objects in creation reverse order
  delete mSysPLL;
  delete mDataGenerator;
  delete mRunControl;
  delete mIPbus;
}

void MBoard::addDataReceiver(int id, MDataReceiver *dr)
{
  if (id + 1 > numReceivers) receivers.resize(id + 1, NULL);
  receivers[id] = dr;
  numReceivers  = id + 1;
}

void MBoard::flushDataReceivers()
{
  for (int i = 0; i < numReceivers; i++)
    if (receivers[i] != NULL) {
      receivers[i]->dataBufferUsed = 0;
      receivers[i]->numClosedData  = 0;
      receivers[i]->dataBufferUsed = 0;
      receivers[i]->flush();
    }
}

void MBoard::connectTCP(int port, int rcvBufferSize)
{
  struct sockaddr_in servaddr;

  closeTCP();

  tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_sockfd == -1) throw MDataConnectError("Socket creation");

  if (rcvBufferSize != 0) {
    // Limit the maximum ammount of "in-flight" data
    // In linux setting the buffer size to 128 KB has the affect of limit
    // the TCP receive window to 162 KB
    if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize, sizeof rcvBufferSize) == -1) {
      closeTCP();
      throw MDataConnectError("setsockopt system call");
    }
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(IPaddress.c_str());
  servaddr.sin_port        = htons(port);

  if (::connect(tcp_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    closeTCP();
    throw MDataConnectError("Can not connect");
  }
}

void MBoard::closeTCP()
{
  if (tcp_sockfd != -1) ::close(tcp_sockfd);
  tcp_sockfd = -1;
  flushDataReceivers();
}

ssize_t MBoard::recvTCP(void *rxBuffer, size_t count)
{
  struct pollfd ufds[2];
  nfds_t        nfds = 0;
  int           rv;
  ssize_t       rxSize;

  ufds[0].fd     = tcp_sockfd;
  ufds[0].events = POLLIN | POLLNVAL; // check for normal read or error
  nfds++;
  if (timer_fd != -1 && !ignoreTimeouts) {
    ufds[nfds].fd     = timer_fd;
    ufds[nfds].events = POLLIN | POLLNVAL;
    nfds++;
  }

  rv = poll(ufds, nfds, TCPtimeout);
  if (rv == -1) throw MDataReceiveError("Poll system call");

  if (rv == 0) {
    if (insideDataPacket) { // TCP connection broken
      throw MDataReceiveError("TCP hanged while reading data");
    }
    else {
      return 0; // normal timeout
    }
  }

  if (!ignoreTimeouts) {
    // check for events on sockfd:
    if (ufds[1].revents & POLLIN) {
      throw MDataReceiveError("Invalid file descriptor in poll system call");
    }
    else {
      if (ufds[1].revents & POLLIN) {
        uint64_t expNum;
        ssize_t  ret = read(timer_fd, &expNum, sizeof(expNum));
        if (ret == 0 || ret == -1) throw MDataReceiveError("MBoard::recvTCP - Error reading timer");
        if (expNum == 0)
          throw MDataReceiveError("MBoard::recvTCP - Timer expiration counter returned 0!");
        // timer expired. Return
        return 0;
      }
    }
  }

  // check for events on sockfd:
  rxSize = 0;
  if (ufds[0].revents & POLLIN) {
    rxSize = recv(tcp_sockfd, rxBuffer, count, 0);
    if (rxSize == 0 || rxSize == -1)
      throw MDataReceiveError("Board connection closed. Fatal error!");
    return rxSize;
  }
  else {
    if (ufds[0].revents & POLLNVAL) {
      throw MDataReceiveError("Invalid file descriptor in poll system call");
    }
  }

  return 0;
}

ssize_t MBoard::readTCPData(void *buffer, size_t count)
{
  ssize_t p = 0;
  ssize_t res;

  while (count) {
    res = recvTCP(buffer, count);

    if (res == 0) return p;
    p += res;
    buffer = (char *)buffer + res;
    count -= res;
    ignoreTimeouts = true;
    TCPtimeout     = TCPhangTimeout; // disable timeout for segments following the first
  }

  return p;
}

unsigned int MBoard::buf2ui(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
  return (*(unsigned int *)buf) & 0xffffffff;
#else
  unsigned int d;

  d = *buf++;
  d |= (*buf++) << 8;
  d |= (*buf++) << 16;
  d |= (*buf++) << 24;

  return d;
#endif
}

#ifdef DEBUG_HEADERS
static void dump(unsigned char *buffer, int size)
{
  int i, j;

  for (i = 0; i < size;) {
    for (j = 0; j < 16; j++) {
      printf(" %02x", buffer[i]);
      i++;
    }
    printf("\n");
  }
}
#endif // DEBUG_HEADERS

//
//	Read data from the TCP socket and dispatch them to the receivers
//
long MBoard::pollTCP(MDataReceiver **drPtr)
{
  const unsigned int bufferSize = 64 * 1024;
  unsigned char      rcvbuffer[bufferSize];
  const unsigned int headerSize = MOSAIC_HEADER_SIZE;
  unsigned char      header[headerSize];
  unsigned int       flags;
  long               blockSize, readBlockSize;
  long               closedDataCounter;
  long               readDataSize = headerSize;
  int                dataSrc;
  ssize_t            n;

  // Read the header
  *drPtr           = NULL;
  ignoreTimeouts   = false;
  insideDataPacket = false;
  n                = readTCPData(header, headerSize);

  if (n == 0) // timeout
    return 0;

  // for all block data after header, disable timer and timeout
  ignoreTimeouts   = true;
  TCPtimeout       = TCPhangTimeout;
  insideDataPacket = true;

#ifdef DEBUG_HEADERS
  printf("Header:\n");
  dump(header, headerSize);
#endif

  blockSize         = buf2ui(header);
  flags             = buf2ui(header + 4);
  closedDataCounter = buf2ui(header + 8);
  dataSrc           = buf2ui(header + 12);

  if (flags & flagOverflow)
    printf("****** Received data block with overflow flag set from source %d\n", dataSrc);

  // round the block size to the higer 64 multiple
  readBlockSize = (blockSize & 0x3f) ? (blockSize & ~0x3f) + 64 : blockSize;
  readDataSize += readBlockSize;

  if (blockSize == 0 && (flags & flagCloseRun) == 0)
    throw MDataReceiveError("Block size set to zero and not CLOSE_RUN");

  // if (flags & flagCloseRun)
  // printf("Received Data packet with CLOSE_RUN\n");

  // skip data from unregistered source
  if (dataSrc > numReceivers || receivers[dataSrc] == NULL) {
    std::cout << "Skipping data block from unregistered source" << std::endl;
    while (readBlockSize) {
      if (readBlockSize > bufferSize)
        n = readTCPData(rcvbuffer, bufferSize);
      else
        n = readTCPData(rcvbuffer, readBlockSize);
      readBlockSize -= n;
    }
    return readDataSize;
  }

  // read data into the consumer buffer
  MDataReceiver *dr = receivers[dataSrc];
  dr->blockFlags    = flags;
  dr->blockSrc      = dataSrc;
  memcpy(dr->blockHeader, header, MOSAIC_HEADER_SIZE);
  *drPtr = dr;

  if (readBlockSize != 0) {
    n = readTCPData(dr->getWritePtr(readBlockSize), readBlockSize);
    if (n == 0) return 0;

    // update the size of data in the buffer
    if (n < blockSize)
      dr->dataBufferUsed += n;
    else
      dr->dataBufferUsed += blockSize;

    // printf("dr->dataBufferUsed: %ld closedDataCounter:%ld flags:0x%04x\n", dr->dataBufferUsed,
  }
  dr->numClosedData += closedDataCounter;

  return readDataSize;
}

//
//	Read data from the TCP socket and send it to the receivers
//
long MBoard::pollData(int timeout)
{
  long           readDataSize;
  long           closedDataCounter;
  MDataReceiver *dr;

  // If all receiver got flagCloseRun, return
  int rcv;
  for (rcv = 0; rcv < numReceivers; rcv++) {
    if ((receivers[rcv]->blockFlags & flagCloseRun) == 0) break;
  }
  if (rcv == numReceivers) {
    // printf("INFO: MBoard::pollData - All receiver got flagCloseRun\n");
    return 0;
  }

  // get data from socket
  TCPtimeout   = timeout;
  readDataSize = pollTCP(&dr);
  if (dr != NULL) {
    closedDataCounter = dr->numClosedData;
    if (closedDataCounter > 0) {
      long parsedBytes = dr->parse(closedDataCounter);

      // move unused bytes to the begin of buffer
      size_t bytesToMove = dr->dataBufferUsed - parsedBytes;
      if (bytesToMove > 0) memmove(&dr->dataBuffer[0], &dr->dataBuffer[parsedBytes], bytesToMove);
      dr->dataBufferUsed -= parsedBytes;
      dr->numClosedData = 0;
    }

    if ((dr->blockFlags & flagCloseRun) && dr->dataBufferUsed != 0) {
      printf("WARNING: MBoard::pollData received data with flagCloseRun but after parsing the "
             "databuffer is not empty (%ld bytes)\n",
             dr->dataBufferUsed);
      //  dump((unsigned char*) &dr->dataBuffer[0], dr->dataBufferUsed);
    }
  }
  return readDataSize;
}

//
//	Read data from the TCP socket and send it to the receivers
//
//  Use this function to poll data for specified time (msec)
//
long MBoard::pollDataTime(int msec)
{
  long readDataSize = 0;

  // create the timer
  timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  if (timer_fd == -1) throw MDataReceiveError("MBoard::pollDataTime - timerfd_create error");

  // arm the timer
  struct itimerspec exp_time;
  exp_time.it_value.tv_sec     = msec / 1000;
  exp_time.it_value.tv_nsec    = (msec % 1000) * 1000;
  exp_time.it_interval.tv_sec  = 0;
  exp_time.it_interval.tv_nsec = 0;

  int res = timerfd_settime(timer_fd, 0, &exp_time, NULL);
  if (res == -1) throw MDataReceiveError("MBoard::pollDataTime - timerfd_settime error");

  // call pollData until timer expires
  for (;;) {
    ignoreTimeouts = false;

    long res = pollData(-1);
    if (res == 0) break;
    readDataSize += res;
  }

  // release kernel resources
  close(timer_fd);
  timer_fd = -1;

  return readDataSize;
}

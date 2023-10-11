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

#ifndef MBOARD_H
#define MBOARD_H

#include "i2cbus.h"
#include "i2csyspll.h"
#include "ipbusudp.h"
#include "mdatagenerator.h"
#include "mruncontrol.h"
#include "mtriggercontrol.h"
#include "mwbb.h"
#include <stdint.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

#define DEFAULT_PACKET_SIZE 1400
#define DEFAULT_UDP_PORT 2000
#define DEFAULT_TCP_BUFFER_SIZE (512 * 1024) // if set to 0 : automatic
#define DEFAULT_TCP_PORT 3333
#define MOSAIC_HEADER_SIZE 64

using namespace std;

class MDataReceiver;

class MBoard {
public:
  MBoard();
  MBoard(const char *IPaddr, int UDPport = DEFAULT_UDP_PORT);
  ~MBoard();

  void setIPaddress(const char *IPaddr, int UDPport = DEFAULT_UDP_PORT);
  void initHardware();
  void connectTCP(int port = DEFAULT_TCP_PORT, int rcvBufferSize = DEFAULT_TCP_BUFFER_SIZE);
  void closeTCP();
  long pollDataTime(int msec);
  long pollData(int timeout);
  void addDataReceiver(int id, MDataReceiver *dc);
  void flushDataReceivers();
  static unsigned int buf2ui(unsigned char *buf);

protected:
  long pollTCP(MDataReceiver **dr);

public:
  MDataGenerator * mDataGenerator;
  IPbusUDP *       mIPbus;
  MRunControl *    mRunControl;
  MTriggerControl *mTriggerControl;
  I2CSysPll *      mSysPLL;

private:
  void    init();
  ssize_t recvTCP(void *buffer, size_t count);
  ssize_t readTCPData(void *buffer, size_t count);

protected:
  int TCPtimeout; // timeout in msec per TCP data reading

private:
  int       timer_fd;
  bool      ignoreTimeouts;
  const int TCPhangTimeout = 2000; // Time in ms after we can consider the TCP connection broken
  bool      insideDataPacket;

public:
  int                          tcp_sockfd;
  int                          numReceivers;
  std::vector<MDataReceiver *> receivers;

public:
  enum dataBlockFlag_e {
    flagClosedEvent = (1 << 0),
    flagOverflow    = (1 << 1),
    flagTimeout     = (1 << 2),
    flagCloseRun    = (1 << 3)
  };

  string IPaddress;
};

#endif // MBOARD_H

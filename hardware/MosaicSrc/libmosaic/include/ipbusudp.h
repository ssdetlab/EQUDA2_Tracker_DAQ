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

#ifndef IPBUSUDP_H
#define IPBUSUDP_H

#include "ipbus.h"
#include <arpa/inet.h>
#include <mutex>
#include <netinet/in.h>
#include <stdint.h>
#include <string>
#include <sys/socket.h>

#define DEFAULT_PACKET_SIZE 1400
#define DEFAULT_PORT 2000
#define RCV_LONG_TIMEOUT 2000 // timeout in ms for the first rx datagrams
#define RCV_SHORT_TIMEOUT 100 // timeout in ms for rx datagrams

class IPbusUDP : public IPbus {
public:
  IPbusUDP(int pktSize = DEFAULT_PACKET_SIZE);
  IPbusUDP(const char *brdName, int port = DEFAULT_PORT, int pktsize = DEFAULT_PACKET_SIZE);
  ~IPbusUDP();
  void              setIPaddress(const char *brdName, int port = DEFAULT_PORT);
  std::string       getIPaddress() { return m_address; };
  void              execute();
  const std::string name() { return "IPbusUDP"; }

private:
  std::string m_address;
  void        testConnection();
  void        sockRead();
  void        sockWrite();

private:
  // int port;
  int                sockfd;
  struct sockaddr_in sockAddress;
  int                rcvTimoutTime;
};

#endif // IPBUSUDP_H

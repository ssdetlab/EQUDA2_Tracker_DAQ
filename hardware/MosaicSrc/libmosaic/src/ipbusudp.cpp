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
#include "ipbusudp.h"
#include "mexception.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>

IPbusUDP::IPbusUDP(int pktSize) : IPbus(pktSize) { sockfd = -1; }

IPbusUDP::IPbusUDP(const char *IPaddr, int port, int pktSize) : IPbus(pktSize)
{
  sockfd = -1;
  setIPaddress(IPaddr, port);
}

void IPbusUDP::setIPaddress(const char *IPaddr, int port)
{
  struct hostent *he;
  m_address = string(IPaddr);

  if ((he = gethostbyname(IPaddr)) == NULL) // get the host address
    throw MIPBusUDPError("Can not resolve board IP address");

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    throw MIPBusUDPError("Can not create socket");

  sockAddress.sin_family = AF_INET;     // host byte order
  sockAddress.sin_port   = htons(port); // short, network byte order
  sockAddress.sin_addr   = *((struct in_addr *)he->h_addr);
  memset(sockAddress.sin_zero, '\0', sizeof sockAddress.sin_zero);

  // Check the connection
  testConnection();
}

IPbusUDP::~IPbusUDP() {}

void IPbusUDP::testConnection()
{
  try {
    rcvTimoutTime = RCV_LONG_TIMEOUT;
    addIdle();
    execute();
    rcvTimoutTime = RCV_SHORT_TIMEOUT;
  }
  catch (MIPBusUDPError &) {
    throw MIPBusUDPError("Board connection error in IPbusUDP::testConnection");
  }
}

void IPbusUDP::sockRead()
{
  struct sockaddr_in peer_addr;
  socklen_t          peer_addr_len;
  struct pollfd      ufds;
  int                rv;

  ufds.fd     = sockfd;
  ufds.events = POLLIN; // check for normal
  rv          = poll(&ufds, 1, rcvTimoutTime);

  if (rv == -1) throw MIPBusUDPError("poll system call");

  if (rv == 0) throw MIPBusUDPTimeout();

  // check for events on sockfd:
  if (ufds.revents & POLLIN) {
    peer_addr_len = sizeof(struct sockaddr);
    rxSize        = recvfrom(sockfd, rxBuffer, getBufferSize(), 0, (struct sockaddr *)&peer_addr,
                      (socklen_t *)&peer_addr_len);
  }

  if (rxSize < 0) throw MIPBusUDPError("Datagram receive system call");
}

void IPbusUDP::sockWrite()
{
  if (sendto(sockfd, txBuffer, txSize, 0, (struct sockaddr *)&sockAddress,
             sizeof(struct sockaddr)) == -1)
    throw MIPBusUDPError("Datagram send system call");
}

void IPbusUDP::execute()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);

  if (txSize == 0) return;

  for (int i = 0; i < 3; i++) {
    try {
      // Send the UDP datagram
      sockWrite();

      // Wait for the answer
      do {
        sockRead();
      } while (duplicatedRxPkt());

      // check the answer packet content
      processAnswer();
      return;
    }
    catch (MIPBusUDPTimeout &) {
      // shoult increase the timeout
      // cout << "Timeout from sockRead" << endl;
    }
  }
  throw MIPBusUDPError("Board comunication error in IPbusUDP::execute");
}

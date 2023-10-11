/*
 * Copyright (C) 2016
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2016.
 *
 */
#include "mservice.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>

#define PKT_ACK 0x06
#define PKT_NAK 0x15

#define CMD_FW_INFO 205

// Service error - Remote Bus Write error
MSrvcError::MSrvcError(const string &arg) { msg = "Service connection Error: " + arg; }

MService::MService()
{
  sockfd    = -1;
  seqNumber = (uint8_t)0;
}

MService::MService(const char *IPaddr, int port)
{
  sockfd = -1;
  setIPaddress(IPaddr, port);
  seqNumber = 0;
}

void MService::setIPaddress(const char *IPaddr, int port)
{
  struct hostent *he;

  if ((he = gethostbyname(IPaddr)) == NULL) // get the host address
    throw MSrvcError("Can not resolve board IP address");

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) throw MSrvcError("Can not create socket");

  sockAddress.sin_family = AF_INET;     // host byte order
  sockAddress.sin_port   = htons(port); // short, network byte order
  sockAddress.sin_addr   = *((struct in_addr *)he->h_addr);
  memset(sockAddress.sin_zero, '\0', sizeof sockAddress.sin_zero);
}

MService::~MService() {}

int MService::sockRead(unsigned char *rxBuffer, int bufSize)
{
  struct sockaddr_in peer_addr;
  socklen_t          peer_addr_len;
  struct pollfd      ufds;
  int                rv;
  int                rxSize = 0;

  ufds.fd     = sockfd;
  ufds.events = POLLIN; // check for normal
  rv          = poll(&ufds, 1, rcvTimoutTime);

  if (rv == -1) throw MSrvcError("Poll system call");

  if (rv == 0) throw MIPBusUDPTimeout();

  // check for events on sockfd:
  if (ufds.revents & POLLIN) {
    peer_addr_len = sizeof(struct sockaddr);
    rxSize        = recvfrom(sockfd, rxBuffer, bufSize, 0, (struct sockaddr *)&peer_addr,
                      (socklen_t *)&peer_addr_len);
  }

  if (rxSize < 0) throw MSrvcError("Datagram receive system call");

  if (rxBuffer[0] != seqNumber) throw MSrvcError("Wrong sequence number");

  if (rxBuffer[1] != PKT_ACK) throw MSrvcError("NACK on response\n");

  return rxSize;
}

void MService::sockWrite(unsigned char *txBuffer, int txSize)
{
  txBuffer[0] = ++seqNumber;
  if (sendto(sockfd, txBuffer, txSize, 0, (struct sockaddr *)&sockAddress,
             sizeof(struct sockaddr)) == -1)
    throw MSrvcError("Datagram send system call");
}

void MService::readFWinfo(fw_info_t *info)
{
  const int     pktSize = 1400;
  unsigned char txBuffer[pktSize];
  int           txSize;
  unsigned char rxBuffer[pktSize];
  ssize_t       nread;
  int           i;

  rcvTimoutTime = RCV_LONG_TIMEOUT;

  /*
          setup the request message
  */
  txSize             = 1; // the sequence number
  txBuffer[txSize++] = CMD_FW_INFO;
  sockWrite(txBuffer, txSize);

  /*
          wait a response from the socket
  */
  nread = sockRead(rxBuffer, pktSize);

  rcvTimoutTime = RCV_SHORT_TIMEOUT;

  if (nread < 8) throw MSrvcError("Response datagram too short");

  i                 = 2;
  info->ver_maj     = rxBuffer[i++];
  info->ver_min     = rxBuffer[i++];
  info->flash_id[0] = rxBuffer[i++];
  info->flash_id[1] = rxBuffer[i++];
  info->flash_id[2] = rxBuffer[i++];

  info->flash_status_register = rxBuffer[i++];

  if (nread > i + 2) {
    memcpy(info->sw_identity, rxBuffer + i, 32);
    info->sw_identity[32] = 0;
    i += 32;
    memcpy(info->fw_identity, rxBuffer + i, 32);
    info->fw_identity[32] = 0;
    i += 32;
  }
  else {
    info->sw_identity[0] = 0;
    info->fw_identity[0] = 0;
  }
}

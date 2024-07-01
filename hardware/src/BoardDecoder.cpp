#include <iostream>

#include "BoardDecoder.h"
#include "TAlpideDataParser.h"

uint32_t BoardDecoder::GetIntFromBinaryString(int numByte, unsigned char *str)
{
  uint32_t number = 0;
  int      pos    = 0;
  int      exp    = numByte - 1;
  while (pos < numByte) {
    number = number + (uint32_t)(str[pos] << 8 * exp);
    exp--;
    pos++;
  }
  return number;
}

uint32_t BoardDecoder::GetIntFromBinaryStringReversed(int numByte, unsigned char *str)
{
  uint32_t number = 0;
  int      pos    = 0;
  while (pos < numByte) {
    number = number + (uint32_t)(str[pos] << 8 * pos);
    pos++;
  }
  return number;
}

// Decodes the Event Header and fill the structure.
// The value of nBytes is filled with the length of read header
bool BoardDecoder::DecodeEventMOSAIC(unsigned char *data, int nBytes, int &nBytesHeader,
                                     int &nBytesTrailer, TBoardHeader &boardInfo)
{
  uint32_t blockFlags = endianAdjust(data + 4);

  boardInfo.overflow = blockFlags & MBoard::flagOverflow;
  boardInfo.endOfRun = blockFlags & MBoard::flagCloseRun;
  boardInfo.timeout  = blockFlags & MBoard::flagTimeout;
  // boardInfo.eoeCount = 1;
  boardInfo.eoeCount = endianAdjust(data + 8);
  boardInfo.channel  = endianAdjust(data + 12) - 1;
  nBytesHeader       = 64; // #define MOSAIC_HEADER_LENGTH 64
  nBytesTrailer      = 1;  // #define The MOSAIC trailer length

  uint8_t MOSAICtransmissionFlag;
  // GDR FIX --- 22/07/2018
  if (nBytes > nBytesHeader + 2) {
    // Not empty frame
    MOSAICtransmissionFlag = data[nBytes - 1]; // last byte is the trailer
    nBytesTrailer          = 1;
  }
  else {
    // CHIP EMPTY FRAME
    MOSAICtransmissionFlag = 0;
    nBytesTrailer          = 0;
  }

  boardInfo.headerError        = MOSAICtransmissionFlag & TAlpideDataParser::flagHeaderError;
  boardInfo.decoder10b8bError  = MOSAICtransmissionFlag & TAlpideDataParser::flagDecoder10b8bError;
  boardInfo.eventOverSizeError = MOSAICtransmissionFlag & TAlpideDataParser::flagOverSizeError;
  if (MOSAICtransmissionFlag) return false;

  return true;
}

// Adapt the (char x 4) -> (unsigned int) conversion depending to the endianess
uint32_t BoardDecoder::endianAdjust(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
  return (*(uint32_t *)buf) & 0xffffffff;
#else
  uint32_t d;
  d = *buf++;
  d |= (*buf++) << 8;
  d |= (*buf++) << 16;
  d |= (*buf++) << 24;
  return d;
#endif
}

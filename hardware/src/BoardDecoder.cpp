#include <iostream>

#include "BoardDecoder.h"
#include "TAlpideDataParser.h"

bool BoardDecoder::DecodeEvent(TBoardType boardType, unsigned char *data, int nBytes,
                               int &nBytesHeader, int &nBytesTrailer, TBoardHeader &boardInfo,
                               uint32_t firmwareVersion, int headerType)
{
  if (boardType == boardDAQ) {
    return DecodeEventDAQ(data, nBytes, nBytesHeader, nBytesTrailer, boardInfo);
  }
  else if (boardType == boardMOSAIC) {
    return DecodeEventMOSAIC(data, nBytes, nBytesHeader, nBytesTrailer, boardInfo);
  }
  else if (boardType == boardRU) {
    return DecodeEventRU(data, nBytes, nBytesHeader, nBytesTrailer, boardInfo);
  }
  else {
    std::cout << "TBoardDecoder: Unknown board type" << std::endl;
    return false;
  }
}

bool BoardDecoder::DecodeEventDAQ(unsigned char *data, int nBytes, int &nBytesHeader,
                                  int &nBytesTrailer, TBoardHeader &boardInfo,
                                  uint32_t firmwareVersion, int headerType)
{

  nBytesHeader =
      BoardDecoder::GetDAQEventHeaderLength(firmwareVersion, headerType); // length in bytes
  nBytesTrailer = BoardDecoder::GetDAQEventTrailerLength();               // length in bytes

  // ------ HEADER

  // bool TDAQBoard::DecodeEventHeader  (unsigned char *data_buf, TEventHeader *AHeader) {
  const int header_length = nBytesHeader / 4; // length in terms of 32-bit words

  int *Header = new int[header_length];
  for (int i = 0; i < header_length; i++) {
    Header[i] = GetIntFromBinaryStringReversed(4, data + i * 4);
    //#ifdef MYDEBUG
    //        std::cout << "Header word: 0x" << std::hex << Header[i] << std:: dec << std::endl;
    //#endif
  }

  // return DecodeEventHeader(Header, length, AHeader);

  // bool  TDAQBoard::DecodeEventHeader  (int *Header, int length, TEventHeader *AHeader){
  // all header words are supposed to have a zero MSB
  for (int i = 0; i < header_length; ++i) {
    if (0x80000000 & Header[i]) {
      std::cout << "Corrupt header data, MSB of header word active!" << std::endl;
      std::cout << std::hex << "0x" << Header[i] << "\t0x" << (0x80000000 & Header[i]) << std::dec
                << std::endl;
      return false;
    }
  }

  bool     AFull             = false;
  int      TrigType          = -1;
  int      BufferDepth       = -1;
  uint64_t Event_ID          = (uint64_t)-1;
  uint64_t TimeStamp         = (uint64_t)-1;
  int      StrobeCountTotal  = (header_length > 5) ? Header[5] : -1;
  int      TrigCountChipBusy = -1;
  int      TrigCountDAQbusy  = -1;
  int      ExtTrigCounter    = -1;
  if (header_length == 3) {
    switch (firmwareVersion) {
    case 0x257E0602:
    case 0x247E0602:
      Event_ID  = (uint64_t)Header[0] & 0x7fffffff;
      TimeStamp = (uint64_t)Header[1] & 0x7fffffff;
      break;
    case 0x257E0610:
    case 0x247E0610:
    default:
      Event_ID = (uint64_t)Header[0] & 0x00ffffff;
      // TimeStamp        = (uint64_t)Header[1] & 0x7fffffff | ((uint64_t)Header[0] & 0x7f000000) <<
      // 7; // Original
      TimeStamp = ((uint64_t)Header[1] & 0x7fffffff) | ((uint64_t)Header[0] & 0x7f000000)
                                                           << 7; // Caterina: added ()
    }
    TrigCountDAQbusy = (Header[2] & 0x7fff0000) >> 8;
    StrobeCountTotal = (Header[2] & 0x00007fff);
  }
  else if (header_length == 5 || header_length == 9) {
    AFull             = (bool)(Header[0] & 0x40);
    TrigType          = (Header[0] & 0x1c00) >> 10;
    BufferDepth       = (Header[0] & 0x1e000) >> 13;
    Event_ID          = ((uint64_t)Header[1] & 0xffffff) | (((uint64_t)Header[2] & 0xffffff) << 24);
    TimeStamp         = ((uint64_t)Header[3] & 0xffffff) | (((uint64_t)Header[4] & 0xffffff) << 24);
    StrobeCountTotal  = (header_length > 5) ? Header[5] : -1;
    TrigCountChipBusy = (header_length > 6) ? Header[6] : -1;
    TrigCountDAQbusy  = (header_length > 7) ? Header[7] : -1;
    ExtTrigCounter    = (header_length > 8) ? Header[8] : -1;

    // few consistency checks:
    if ((Header[0] & 0xfffe03bf) != 0x8) {
      std::cout << "Corrupt header word 0: 0x" << std::hex << Header[0] << std::dec << std::endl;
      return false;
    }
    if ((Header[1] & 0xff000000) || (Header[2] & 0xff000000) || (Header[3] & 0xff000000)) {
      std::cout << "Corrupt header, missing at least one of the leading 0s in word 1-4"
                << std::endl;
      return false;
    }
    if ((TrigType < 1) || (TrigType > 2)) {
      std::cout << "Bad Trigger Type " << TrigType << std::endl;
      return false;
    }
  }
  //#ifdef MYDEBUG
  //    std::cout << "Header: Trigger type = " << TrigType << std::endl;
  //    std::cout << "Header: Almost full  = " << (int) AFull << std::endl;
  //    std::cout << "Header: Event ID     = " << Event_ID << std::endl;
  //    std::cout << "Header: Time stamp   = " << TimeStamp << std::endl;
  //    std::cout << "             in sec  = " << (float)TimeStamp / 8e7 << std::endl;
  //    std::cout << "Header: Total Strobe Count       = " << StrobeCountTotal << std::endl;
  //    std::cout << "Header: Trigger Count Chip Busy  = " << TrigCountChipBusy << std::endl;
  //    std::cout << "Header: Trigger Count DAQ Busy   = " << TrigCountDAQbusy << std::endl;
  //    std::cout << "Header: External Trigger Count   = " << ExtTrigCounter << std::endl;
  //#endif

  boardInfo.almostFull        = AFull;
  boardInfo.bufferDepth       = BufferDepth;
  boardInfo.eventId           = Event_ID;
  boardInfo.timestamp         = TimeStamp;
  boardInfo.trigType          = TrigType;
  boardInfo.strobeCount       = StrobeCountTotal;
  boardInfo.trigCountChipBusy = TrigCountChipBusy;
  boardInfo.trigCountDAQBusy  = TrigCountDAQbusy;
  boardInfo.extTrigCount      = ExtTrigCounter;
  boardInfo.channel           = 0; // match this value to SetupHelpers.cpp / initSetupSingle

  // TRAILER

  // bool TDAQBoard::DecodeEventTrailer (unsigned char *data_buf, TEventHeader *AHeader) {
  unsigned int Trailer[2];
  for (int i = 0; i < 2; i++) {
    Trailer[i] = GetIntFromBinaryStringReversed(4, (data + nBytes - nBytesTrailer) + i * 4);
    //#ifdef MYDEBUG
    //        std::cout << "Trailer word: 0x" << std::hex << Trailer[i] << std:: dec << std::endl;
    //#endif
  }

  //    return DecodeEventTrailer(Trailer, AHeader);
  // bool TDAQBoard::DecodeEventTrailer (int * Trailer, TEventHeader *ATrailer) {
  if (Trailer[1] != DAQ_TRAILER_WORD) {
    std::cout << "Corrupt trailer, expecting 0x " << std::hex << DAQ_TRAILER_WORD << ", found 0x"
              << Trailer[1] << std::dec << std::endl;
    return false;
  }
  int EventSize = Trailer[0];

  boardInfo.eventSize = EventSize;
  //#ifdef MYDEBUG
  //  std::cout << "Trailer: Event size = " << EventSize << std::endl;
  //  std::cout << std::hex<< "Trailer: 2 word = " << Trailer[1] <<  std::dec << std::endl;
  //#endif

  delete[] Header;

  return true;
}

int BoardDecoder::GetDAQEventHeaderLength(uint32_t firmwareVersion, int headerType)
{
  switch (firmwareVersion) {
  case 0x257E030A:
  case 0x247E030A:
  case 0x257E031D:
  case 0x247E031D:
    return 36;
    break;
  case 0:
    return -1;
    break;
  case 0x257E0602:
  case 0x247E0602:
  case 0x257E0610:
  case 0x247E0610:
    return 12;
    break;
  case 0x247E0611:
  case 0x347E0803:
    return (headerType == 0) ? 36 : 12;
    break;
  default:
    return 20;
    break;
  }
  return 20;
}

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

bool BoardDecoder::DecodeEventRU(unsigned char *data, int nBytes, int &nBytesHeader,
                                 int &nBytesTrailer, TBoardHeader &boardInfo)
{
  nBytesHeader  = 0;
  nBytesTrailer = 1;

  unsigned char chipId = data[nBytes - 1]; // last byte is the transiver number
  boardInfo.channel    = (int)chipId;
  return true;
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

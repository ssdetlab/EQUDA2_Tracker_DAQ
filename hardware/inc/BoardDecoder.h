#ifndef BOARDDECODER_H
#define BOARDDECODER_H

#include "TReadoutBoard.h"

const uint32_t DAQ_TRAILER_WORD = 0xbfbfbfbf; //

// put all header and trailer information here
// (both for mosaic and DAQ board)
typedef struct {
  // common
  //  int  size;
  // MOSAIC
  int  channel;
  int  eoeCount;
  bool timeout;
  bool endOfRun;
  bool overflow;
  bool headerError;        // the received Frame contains error in the transmission
  bool decoder10b8bError;  // the MOSAIC board reports a 10b8b conversion error
  bool eventOverSizeError; // the MOSAIC board reports an Event Over Size Error

  // DAQ board
  bool     almostFull;
  int      trigType;
  int      bufferDepth;
  uint64_t eventId;
  uint64_t timestamp;
  int      eventSize;
  bool     truncated;
  int      strobeCount;
  int      trigCountChipBusy;
  int      trigCountDAQBusy;
  int      extTrigCount;
} TBoardHeader;

// Board Decoder decodes the information contained in the readout board header and trailer
// the information is written into TBoardHeader (one common structure for DAQ board and readout
// board)
// data and nBytes are modified such that after the board decoding they correspond to the chip event
// only
class BoardDecoder {
private:
  static bool DecodeEventRU(unsigned char *data, int nBytes, int &nBytesHeader, int &nBytesTrailer,
                            TBoardHeader &boardInfo);
  static uint32_t endianAdjust(unsigned char *buf);

  static bool DecodeEventDAQ(unsigned char *data, int nBytes, int &nBytesHeader, int &nBytesTrailer,
                             TBoardHeader &boardInfo, uint32_t firmwareVersion = 0x247E0611,
                             int headerType = 0x1);
  static uint32_t GetIntFromBinaryString(int numByte, unsigned char *str);
  static uint32_t GetIntFromBinaryStringReversed(int numByte, unsigned char *str);

public:
  static bool DecodeEventMOSAIC(unsigned char *data, int nBytes, int &nBytesHeader,
                                int &nBytesTrailer, TBoardHeader &boardInfo);

  static bool DecodeEvent(TBoardType boardType, unsigned char *data, int nBytes, int &nBytesHeader,
                          int &nBytesTrailer, TBoardHeader &boardInfo,
                          uint32_t firmwareVersion = 0x247E0611, int headerType = 0x1);
  static int  GetDAQEventHeaderLength(uint32_t firmwareVersion = 0x247E0611, int headerType = 1);
  static int  GetDAQEventTrailerLength() { return 8; };
};

#endif

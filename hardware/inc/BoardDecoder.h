#ifndef BOARDDECODER_H
#define BOARDDECODER_H

#include "TReadoutBoard.h"

const uint32_t DAQ_TRAILER_WORD = 0xbfbfbfbf; //

// put all header and trailer information here
typedef struct {
    int  channel;
    int  eoeCount;
    bool timeout;
    bool endOfRun;
    bool overflow;
    bool headerError;        // the received Frame contains error in the transmission
    bool decoder10b8bError;  // the MOSAIC board reports a 10b8b conversion error
    bool eventOverSizeError; // the MOSAIC board reports an Event Over Size Error
} TBoardHeader;

// Board Decoder decodes the information contained 
// in the readout board header and trailer
//
// The information is written into TBoardHeader 
//
// Data and nBytes are modified such that after the 
// board decoding they correspond to the chip event only
class BoardDecoder {
    private:
        static uint32_t endianAdjust(unsigned char *buf);

        static uint32_t GetIntFromBinaryString(int numByte, unsigned char *str);
        static uint32_t GetIntFromBinaryStringReversed(int numByte, unsigned char *str);

    public:
        static bool DecodeEventMOSAIC(
            unsigned char *data, int nBytes, int &nBytesHeader,
            int &nBytesTrailer, TBoardHeader &boardInfo);
};

#endif

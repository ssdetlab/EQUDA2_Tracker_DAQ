#ifndef ALPIDEDEBUG_H
#define ALPIDEDEBUG_H

#include "TAlpide.h"

namespace Alpide {

  typedef struct {
    int  BusyRequestSM;
    int  BusyGeneratorSM;
    bool BusyInState;
    bool SEUErrorOR;
  } TBMUDebugStream;

  typedef struct {
    int  DataFIFOReadPointer[3];
    int  DataFIFOWritePointer[3];
    int  BusyFIFOReadPointer;
    int  BusyFIFOWritePointer;
    int  LocalBusValue;
    int  LocalBusOENGenSM;
    int  MuxFIFOCtrlSM;
    int  CtrlWordDecoderLocalBusSM;
    int  TokenGrantorSM;
    int  SendCommaSM;
    int  DataPackingSM;
    bool SEUErrorOR;
    bool BusyMismatchError;
    bool BusyFIFOError;
    bool DataFIFOError;
  } TDMUDebugStream;

  typedef struct {
    int  FrameStartFIFOReadPointer[3];
    int  FrameStartFIFOWritePointer[3];
    int  FrameEndFIFOReadPointer;
    int  FrameEndFIFOWritePointer;
    int  TRUSM;
    int  ClockEnableSM;
    bool SEUErrorOR;
    bool FrameStartFIFOError;
    bool FrameEndFIFOError;
  } TTRUDebugStream;

  typedef struct {
    int  MEBFIFOReadPointer[32];
    int  MEBFIFOWritePointer[32];
    int  MEBFIFOSM[32];
    bool SEUErrorOR[32];
    int  RegionReadoutSM[32];
    int  RegionValidDM[32];
    int  GenRgnSM[32];
    int  FIFOSelfTestSM[32];
    int  RoClockEnableSM[32];
    int  TruClockEnableSM[32];
    int  CfgClockEnableSM[32];
    int  FtClockEnableSM[32];
  } TRRUDebugStream;

  typedef struct {
    bool SEUErrorOR;
    int  BunchCounter;
    int  TriggerCounter;
    int  StrobeCounter;
    int  FrameCounter;
    int  ReadoutCounter;
    // int  BunchCounter;
    int StrobeManagerSM;
    int FlushValue;
    int WriterSM;
    int ReaderSM;
    int PRSTSM;
    int StrobeValue;
    int MemselValue;
    int BusyManagerSM;
    int EventInMEB;
  } TFromuDebugStream;

  typedef struct {
    bool ComparatorOut;
    int  SampledValue;
    int  SelectDACRow;
    int  SelectDACColumn;
    int  ADCSM;
    int  ADCSequencerState;
    bool SEUErrorOR;
  } TADCDebugStream;
} // namespace Alpide

using namespace Alpide;

namespace AlpideDebug {
  bool ReadStream(TAlpide *chip, TRegister reg, uint16_t *stream, int len, uint16_t Header);
  bool GetBMUDebugStream(TAlpide *chip, TBMUDebugStream &stream);
  bool GetDMUDebugStream(TAlpide *chip, TDMUDebugStream &stream);
  bool GetTRUDebugStream(TAlpide *chip, TTRUDebugStream &stream);
  bool GetRRUDebugSteam(TAlpide *chip, TRRUDebugStream &stream);
  bool GetFromuDebugStream(TAlpide *chip, TFromuDebugStream &stream);
  bool GetADCDebugStream(TAlpide *chip, TADCDebugStream &stream);
} // namespace AlpideDebug

#endif

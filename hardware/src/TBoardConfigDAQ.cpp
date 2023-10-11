#include "TBoardConfigDAQ.h"

TBoardConfigDAQ::TBoardConfigDAQ(const char *fName, int boardIndex)
{
  // fill default value from header file
  ////---- ADC module

  fBoardType = boardDAQ;
  // ADC config reg 0 !! values as found in old TDaqBoard::PowerOn()
  fCurrentLimitDigital = LIMIT_DIGITAL;
  fCurrentLimitIo      = LIMIT_IO;
  fAutoShutdownEnable  = true;
  fLDOEnable           = false;
  fADCEnable           = false;
  fADCSelfStop         = false;
  fDisableTstmpReset   = true;
  fPktBasedROEnableADC = false;
  // ADC config reg 1
  fCurrentLimitAnalogue = LIMIT_ANALOGUE;
  // ADC config reg 2
  fAutoShutOffDelay    = 0;
  fADCDownSamplingFact = 0;

  ////---- READOUT module

  // Event builder config reg
  fMaxDiffTriggers = 1; // TODO: check if any influence
  fSamplingEdgeSelect =
      DATA_SAMPLING_EDGE;                // 0: positive edge, 1: negative edge (for pA1 inverted..)
  fPktBasedROEnable  = DATA_PKTBASED_EN; // 0: disable, 1: enable
  fDDREnable         = DATA_DDR_EN;      // 0: disable, 1: enable
  fDataPortSelect    = DATA_PORT;        // 01: serial port, 10: parallel port
  fFPGAEmulationMode = 0;                // 00: FPGA is bus master
  fHeaderType        = HEADER_TYPE;
  fBoardVersion      = BOARD_VERSION;

  ////---- TRIGGER module

  // Busy configuration register
  // uint32_t fBusyDuration = 4;
  // Trigger configuration register
  fNTriggers      = 1; // TODO: feature ever used?
  fTriggerMode    = TRIGGER_MODE;
  fStrobeDuration = 10; // depreciated
  fBusyConfig     = BUSY_CONFIG;
  // Strobe delay register
  fStrobeDelay = STROBE_DELAY;
  // Busy override register
  fBusyOverride = BUSY_OVERRIDE;

  ////---- CMU module

  // CMU config register
  fManchesterDisable     = true;  // 0: enable manchester encoding; 1: disable
  fSamplingEdgeSelectCMU = true;  // 0: positive edge; 1: negative edge
  fInvertCMUBus          = false; // 0: bus not inverted; 1: bus inverted
  fChipMaster            = false; // 0: chip is master; 1: chip is slave

  ////---- RESET module

  // PULSE DRST PRST duration reg
  fPRSTDuration  = 10; // depreciated
  fDRSTDuration  = 10; // TODO: should rather be done via opcode?
  fPULSEDuration = 10; // depreciated
  // Power up sequencer delay reg
  fAutoShutdownTime = AUTOSHTDWN_TIME;
  fClockEnableTime  = CLOCK_ENABLE_TIME;
  fSignalEnableTime = SIGNAL_ENABLE_TIME;
  fDrstTime         = DRST_TIME;
  // PULSE STROBE delay sequence reg
  fPulseDelay     = PULSE_STROBE_DELAY;
  fStrobePulseSeq = STROBE_PULSE_SEQ;
  // PowerOnReset disable reg
  fPORDisable = true; //     0; 0: enable POR; 1: disable

  ////---- ID module
  fBoardAddress = -1; // accept any address

  ////---- SOFTRESET module

  // Software reset duration register
  fSoftResetDuration = 10;

  InitParamMap();
}

void TBoardConfigDAQ::InitParamMap()
{
  TBoardConfig::InitParamMap();
  fSettings["BOARDVERSION"] = &fBoardVersion;
}

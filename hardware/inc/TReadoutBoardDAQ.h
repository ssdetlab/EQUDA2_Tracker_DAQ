//
//  TDaqboard.h
//

#ifndef READOUTBOARDDAQ_H
#define READOUTBOARDDAQ_H

#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "BoardDecoder.h"
#include "TAlpide.h"
#include "TBoardConfigDAQ.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "USB.h"

// enum TTriggerSource {TRIG_INT, TRIG_EXT};

const int MAX_DIFF_TRIG_EVT_CNT = 10; // maximum allowed difference between number triggers and
                                      // events read; MAX_DIFF_TRIG_EVT_CNT is default
const uint32_t MAX_EVT_BUFFSIZE =
    1e3; // max number of events in fEventBuffer  TODO: maximum queue size ~1 Gb?
const int MAX_NTRIG_TRAIN = 10; // fNTriggers will be subdivided into trigger trains with
                                // fMaxNTriggersAtOnce, MAX_NTRIG_ATONCE is default

//************************************************************
// TReadOutBoardDAQ: implementationn for Cagliari DAQboard
//************************************************************

class TReadoutBoardDAQ : public TUSBBoard, public TReadoutBoard {
private:
  static const int NEndpoints         = 4;
  static const int ENDPOINT_WRITE_REG = 0;
  static const int ENDPOINT_READ_REG  = 1;
  static const int ENDPOINT_READ_ADC  = 2;
  static const int ENDPOINT_READ_DATA = 3;

  // instruction words
  static const int DAQBOARD_WORD_SIZE =
      4; // communication to DAQboard based on 32-bit words, 4 bytes
  static const int DAQBOARD_REG_ADDR_SIZE    = 8; // sub(reg)-address size = 12-bit
  static const int DAQBOARD_MODULE_ADDR_SIZE = 4; // module-address size   =  4-bit

  //// Cagliari DAQ board register description
  //---------------------------------------------------------

  /// module addresses
  static const int MODULE_CONTROL = 0x0;
  static const int MODULE_ADC     = 0x1;
  static const int MODULE_READOUT = 0x2;
  static const int MODULE_TRIGGER = 0x3;
  // static const int MODULE_JTAG      = 0x4;
  static const int MODULE_CMU       = 0x4;
  static const int MODULE_RESET     = 0x5;
  static const int MODULE_IDENT     = 0x6;
  static const int MODULE_ID        = 0x6;
  static const int MODULE_SOFTRESET = 0x7;

  // ADC Module 0x1: Register sub-addresses
  static const int ADC_CONFIG0  = 0x0;
  static const int ADC_CONFIG1  = 0x1;
  static const int ADC_CONFIG2  = 0x2;
  static const int ADC_DATA0    = 0x3; // Read only, previously ADC_READ0
  static const int ADC_DATA1    = 0x4; // Read only, previously ADC_READ1
  static const int ADC_DATA2    = 0x5; // Read only, previously ADC_READ2
  static const int ADC_OVERFLOW = 0x9; // Read only

  // READOUT Module 0x2: Register sub-addresses
  static const int READOUT_EVENTBUILDER_CONFIG = 0x0; // previously   READOUT_CHIP_DATA
  static const int READOUT_EOR_COMMAND         = 0x1; // previously   READOUT_ENDOFRUN
  static const int READOUT_EVTID1              = 0x2; //
  static const int READOUT_EVTID2              = 0x3; //
  static const int READOUT_RESYNC              = 0x4; //
  static const int READOUT_SLAVE_DATA_EMULATOR = 0x5; //
  static const int READOUT_TIMESTAMP1          = 0x6; // not existing in manual..
  static const int READOUT_TIMESTAMP2          = 0x7; // not existing in manual..
  static const int READOUT_MONITOR1            = 0x8; // not existing in manual..

  // TRIGGER Module 0x3: Register sub-addresses
  static const int TRIG_BUSY_DURATION  = 0x0;
  static const int TRIG_TRIGGER_CONFIG = 0x1;
  static const int TRIG_START          = 0x2;
  static const int TRIG_STOP           = 0x3;
  static const int TRIG_DELAY          = 0x4;
  static const int TRIG_BUSY_OVERRIDE  = 0x5;
  static const int TRIG_STROBE_COUNT   = 0x6; // not existing in manual..
  static const int TRIG_MONITOR1       = 0x7; // not existing in manual..

  // CMU Module 0x4: Register sub-addresses
  static const int CMU_INSTR  = 0x0; // previously called DAQBOARD_WRITE_INSTR_REG from JTAG?
  static const int CMU_DATA   = 0x1; // previously called DAQBOARD_WRITE_DATA_REG from JTAG?
  static const int CMU_CONFIG = 0x2; // previously called DAQ_CONFIG_REG
                                     //// JTAG Module 0x4: Register TODO -> ONLY FOR PALPIDE-1??
  // static const int DAQBOARD_WRITE_INSTR_REG  = 0x0;
  // static const int DAQBOARD_WRITE_DATA_REG   = 0x1;

  // RESET Module 0x5: Register sub-addresses
  static const int RESET_DURATION = 0x0; // PULSE and PRST duration only used in pA1, became OPCODEs
                                         // in later verstions; (D)RST using CMU interface for later
                                         // versions? DRST == GRST!
  static const int RESET_DELAYS      = 0x1;
  static const int RESET_DRST        = 0x2;
  static const int RESET_PRST        = 0x3;
  static const int RESET_PULSE       = 0x4;
  static const int RESET_PULSE_DELAY = 0x5;
  static const int RESET_POR_DISABLE = 0x6;

  // IDENTIFICATION Module 0x6: Register sub-addresses
  static const int ID_ADDRESS     = 0x0;
  static const int ID_CHIP        = 0x1;
  static const int ID_FIRMWARE    = 0x2;
  static const int ID_ACK_COUNTER = 0x3;

  // SOFTRESET Module Register 0x7: Register sub-addresses
  static const int SOFTRESET_DURATION = 0x0;
  // static const int SOFTRESET_COMMAND    = 0x1; // previously SOFTRESET_FPGA_RESET?
  static const int SOFTRESET_FPGA_RESET = 0x1; // not existing in manual..
  static const int SOFTRESET_FX3_RESET  = 0x2; // not existing in manual..

  //--------------------------------------

  uint32_t fFirmwareVersion;

  TBoardConfigDAQ *fBoardConfigDAQ;

  int SendWord(uint32_t value);
  int ReadAcknowledge();

  int WriteChipRegister(uint16_t address, uint16_t value, TAlpide *chipPtr = 0);
  int ReadChipRegister(uint16_t address, uint16_t &value, TAlpide *chipPtr = 0);

  // members and methods related to data readout
  void DAQTrigger();   // function for triggering fNTrigger events, to be ran in thread
  int  fStatusTrigger; // status variable for trigger
                       //  1: no error
                       //  -1: fMaxEventBufferSize reached
  void DAQReadData();  // function to read raw data, split into DAQboard events, and writing events
                       // to fEventBuffer
  int fStatusReadData; // status variable for readdata
                       //  1: no error
                       // -1: general errror, not specifically treated
                       // -2: USB timeout
                       // -3: stop trigger marker
  std::thread fThreadTrigger;           // thread for DAQTrigger
  bool        fIsTriggerThreadRunning;  // boolan to check if thread is still running
  int         fTrigCnt;                 // overall trigger counter
  std::thread fThreadReadData;          // thread for DAQReadData
  bool        fIsReadDataThreadRunning; // boolan to check if thread is still running
  int         fEvtCnt;                  // counter of events read/in queue
  int         fDiffTrigEvtCnt;          // difference between number triggers and events read
  int      fMaxDiffTrigEvtCnt; // maximum allowed difference between number triggers and events read
  uint32_t fMaxEventBufferSize; // maximum number of events in fEventBuffer; TODO: maximum queue
                                // size ~1 Gb?
  int fNTriggersTotal;          // total number of triggers to be launched
  int fMaxNTriggersTrain;       // fNTriggersTotal will be subdivided into trigger trains with
                                // fMaxNTriggersAtOnce
  std::mutex fMtx;              //  mutex for read/write acces to fEventBuffer

  std::deque<std::vector<unsigned char>> fEventBuffer; // double ended queue for DAQboard event
                                                       // data; vector<unsigned char> for saving
                                                       // events
  std::deque<unsigned char> fRawBuffer;                // double ended queue for raw data;

protected:
public:
  TReadoutBoardDAQ(libusb_device *ADevice, TBoardConfigDAQ *config);

  virtual ~TReadoutBoardDAQ();

  void DumpConfig(const char *fName, bool writeFile = true, char *config = 0);

  //// general methods of TReadoutBoard
  //---------------------------------------------------------

  int ReadRegister(uint16_t address, uint32_t &value);
  int WriteRegister(uint16_t address, uint32_t value);

  int SendOpCode(Alpide::TOpCode OpCode);
  // DAQ board has only one control interface -> both methods are identical
  int SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr)
  {
    (void)chipPtr;
    return SendOpCode(OpCode);
  };

  int SendCommand(Alpide::TCommand Command, TAlpide *chipPtr);

  int  SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
  void SetTriggerSource(TTriggerSource triggerSource);
  int  Trigger(int nTriggers);
  int  ReadEventData(int &nBytes, unsigned char *buffer);

  //// methods only for Cagliari DAQ board
  //---------------------------------------------------------
  TBoardConfigDAQ *GetBoardConfig() { return fBoardConfigDAQ; };

  bool PowerOn(int &overflow);
  void PowerOff();

  void ReadAllRegisters();

  int   CurrentToADC(int current);
  float ADCToSupplyCurrent(int value);
  float ADCToDacmonCurrent(int value);
  float ADCToTemperature(int value);

  bool ReadMonitorRegisters();
  bool ReadMonitorReadoutRegister();
  bool ReadMonitorTriggerRegister();

  //// methods related to data readout
  //---------------------------------------------------------
  int GetEventBufferLength();
  int GetStatusReadData() { return fStatusReadData; }

  // methods related to modules of Cagliari DAQ board
  //--------------------------------------

  // ADC Module:
  float ReadAnalogI();     // read analogue supply current
  float ReadDigitalI();    // read digital supply current
  float ReadIoI();         // read digital I/O supply current
  float ReadTemperature(); // read temperature

  float ReadMonI(); //
  float ReadMonV(); //

  bool ReadLDOStatus(int &overflow);
  void DecodeOverflow(int overflow);

  void
  WriteADCModuleConfigRegisters(); // write current ADC module config (fBoardConfigDAQ) to registers
  // int  WriteADCConfig      ();
  void WriteCurrentLimits(bool LDOOn, bool autoshutdown); // just write current limits

  // READOUT Module:
  void WriteReadoutModuleConfigRegisters(); // write current Readout module config (fBoardConfigDAQ)
                                            // to registers
  bool ResyncSerialPort();
  bool WriteSlaveDataEmulatorReg(uint32_t data);
  bool EndOfRun()
  {
    return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_EOR_COMMAND, 5);
  };

  void StartRun(){};
  // TRIGGER Module:
  bool StartTrigger();
  bool StopTrigger();

  void WriteTriggerModuleConfigRegisters(); // write current trigger module config (fBoardConfigDAQ)
                                            // to registers
  bool WriteBusyOverrideReg(bool busyOverride);

  // CMU Module:
  void
  WriteCMUModuleConfigRegisters(); // write current CMU module config (fBoardConfigDAQ) to registers

  // RESET Module:
  void WriteResetModuleConfigRegisters(); // write current reset module config (fBoardConfigDAQ) to
                                          // registers
  void WriteDelays();

  // ID Module:
  int      ReadBoardAddress();
  bool     CheckBoardAddress();
  uint32_t ReadFirmwareVersion();
  uint32_t ReadFirmwareDate();
  int      ReadFirmwareChipVersion();

  // SOFTRESET Module:
  void WriteSoftResetModuleConfigRegisters(); // write current soft reset module config
                                              // (fBoardConfigDAQ) to registers
  bool ResetBoardFPGA(int duration);
  bool ResetBoardFX3(int duration);
};

//************************************************************

#endif /* READOUTBOARDDAQ_H */

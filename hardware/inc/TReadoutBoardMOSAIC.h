#ifndef READOUTBOARDMOSAIC_H
#define READOUTBOARDMOSAIC_H

#include <deque>
#include <exception>
#include <string>

#include "BoardDecoder.h"
#include "Mosaic.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "powerboard.h"
#include "trgrecorderparser.h"

// Constant Definitions
#define DEFAULT_PACKET_SIZE 1400
#define DEFAULT_UDP_PORT 2000
#define DEFAULT_TCP_BUFFER_SIZE (512 * 1024) // if set to 0 : automatic
#define DEFAULT_TCP_PORT 3333

#define DATA_INPUT_BUFFER_SIZE 64 * 1024


using namespace std;

extern std::vector<unsigned char> fDebugBuffer;

class DummyReceiver : public MDataReceiver {
public:
  DummyReceiver(){};
  ~DummyReceiver(){};

  int  ReadEventData(int &nBytes, unsigned char *buffer);
  long parse(int numClosed)
  {
    (void)numClosed;
    return (dataBufferUsed);
  };
};

class TReadoutBoardMOSAIC : public TReadoutBoard, private MBoard{
  // Methods
  public:
    TReadoutBoardMOSAIC(TConfig *config, TBoardConfigMOSAIC *boardConfig);
    virtual ~TReadoutBoardMOSAIC();

    int WriteChipRegister(uint16_t address, uint16_t value, TAlpide *chipPtr);
    int ReadChipRegister(uint16_t address, uint16_t &value, TAlpide *chipPtr);
    int SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr);
    int SendOpCode(Alpide::TOpCode OpCode);
    int SendCommand(Alpide::TCommand Command, TAlpide *chipPtr);
  
    int      SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
    void     SetTriggerSource(TTriggerSource triggerSource);
    uint32_t GetTriggerCount();
    int      Trigger(int nTriggers);
  
    int  ReadEventData(int &nBytes, unsigned char *buffer);
    void StartRun();
    void StopRun();

    int ReadRegister(uint16_t Address, uint32_t &Value){
      (void)Address;
      (void)Value;
      return 0;
    };
    int WriteRegister(uint16_t Address, uint32_t Value){
      (void)Address;
      (void)Value;
      return 0;
    };

    void WriteChipRegionRegister(uint16_t rgn_addr,
                                 uint16_t base_addr,
                                 uint16_t reg_addr,
                                 uint16_t val,
                                 TAlpide* chipPtr){
      WriteChipRegister( (rgn_addr & 0x1F) << 11 | (base_addr & 0x7) << 8 | reg_addr & 0xFF, val, chipPtr);
    }    

    void enableControlInterfaces(bool en);
    void enableControlInterface(int interface, bool en);
    void enableClockOutput(int interface, bool en){
      enableControlInterface(interface, en);
      return;
    }
    void setInverted(bool AInverted, int Aindex = -1);

    bool PowerOn();
    void PowerOff();

    int           GetFwMajVersion(){ return theVersionMaj; };
    int           GetFwMinVersion(){ return theVersionMin; };
    char*         GetFwIdString()  { return theVersionId; };
    
    powerboard*   GetPowerBoardHandle() { return pb; };
    MCoordinator* GetCoordinatorHandle(){ return coordinator; };
    std::string   GetRegisterDump();
    
    void setSpeedMode(Mosaic::TReceiverSpeed ASpeed, int Aindex = -1);
    void WriteTransceiverDRP(size_t Aindex, uint16_t address, uint16_t value, bool execute = true);
    void WriteTransceiverDRPField(size_t Aindex, uint16_t address, uint16_t size, uint16_t offset,
                                  uint16_t value, bool execute = true);
    void ReadTransceiverDRP(size_t Aindex, uint16_t address, uint32_t *value, bool execute = true);

    void setReadTriggerInfo(bool readTriggerInfo = true);

    std::vector<uint32_t> *getTriggerNums() { return &triggerNum; };
    std::vector<uint64_t> *getTriggerTimes() { return &triggerTime; };

    void     SetReceiverPatternCheck(size_t Aindex);
    void     ResetReceiverPatternCheck(size_t Aindex);
    uint32_t GetErrorCounter(size_t Aindex);
    void     ResetReceiver(size_t Areceiver);
    void     ResetAllReceivers();
  
  
    void DumpTriggerControl();
  private:
    void init();
    void waitResetDone();
    void enableDefinedReceivers();
    void setPhase(int APhase, int ACii = 0){
      controlInterface[ACii]->setPhase(APhase);
      controlInterface[ACii]->addSendCmd(ControlInterface::OPCODE_GRST);
      controlInterface[ACii]->execute();
      return;
    };

    uint32_t decodeError();
    char *   getFirmwareVersion();

  // Properties
  private:
    TBoardConfigMOSAIC *fBoardConfig;
    I2Cbus *           i2cBus;
    I2Cbus *           i2cBusAux;
    powerboard *       pb;
    ControlInterface * controlInterface[MAX_MOSAICCTRLINT];
    Pulser *           pulser;
    ALPIDErcv *        alpideRcv[MAX_MOSAICTRANRECV];
    TAlpideDataParser *alpideDataParser[MAX_MOSAICTRANRECV];
    DummyReceiver *    dr;

    char               theVersionId[50]; // Version properties
    int                theVersionMaj;
    int                theVersionMin;
    TrgRecorder *      trgRecorder;
    TrgRecorderParser *trgDataParser;
    MCoordinator *     coordinator;

    bool                  readTriggerInfo;
    std::vector<uint32_t> triggerNum;
    std::vector<uint64_t> triggerTime;

    // extend WBB address definitions in mwbb.h
    enum baseAddress_e{
      add_i2cMaster         = (5 << 24),
      add_controlInterface  = (6 << 24),
      add_controlInterfaceB = (7 << 24),
      add_alpideRcv         = (8 << 24),
      // total of 10 alpideRcv

      add_trgRecorder        = (18 << 24),
      add_controlInterface_0 = (19 << 24),
      add_controlInterface_9 = (28 << 24),
      add_i2cAux             = (29 << 24),
      add_coordinator        = (30 << 24)
    };

    // status register bits
    enum BOARD_STATUS_BITS {
      BOARD_STATUS_FEPLL_LOCK     = 0x0001,
      BOARD_STATUS_EXTPLL_LOCK    = 0x0002,
      BOARD_STATUS_GTPLL_LOCK     = 0x0004,
      BOARD_STATUS_GTP_RESET_DONE = 0x3ff0000
    };

    enum configBits_e {
      CFG_EXTCLOCK_SEL_BIT = (1 << 0), // 0: internal clock - 1: external clock
      CFG_CLOCK_20MHZ_BIT  = (1 << 1), // 0: 40 MHz clock	- 1: 20 MHz clock
      CFG_RATE_MASK        = (0x03 << 2),
      CFG_RATE_1200        = (0 << 2),
      CFG_RATE_600         = (0x01 << 2),
      CFG_RATE_400         = (0x02 << 2)
    };

    static I2CSysPll::pllRegisters_t sysPLLregContent;
};
#endif
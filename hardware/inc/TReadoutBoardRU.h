#ifndef TREADOUTBOARDRU_H
#define TREADOUTBOARDRU_H

#include "TBoardConfigRU.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "USB.h"

#include <deque>
#include <map>
#include <memory>

#include "ReadoutUnitSrc/TRuDctrlModule.h"
#include "ReadoutUnitSrc/TRuTransceiverModule.h"
#include "ReadoutUnitSrc/UsbDev.hpp"

class TReadoutBoardRU : public TReadoutBoard {
public:
  struct ReadResult {
    uint16_t address;
    uint16_t data;
    bool     error;
    ReadResult(uint16_t address, uint16_t data, bool error)
        : address(address), data(data), error(error)
    {
    }
  };

  static const int     VID;
  static const int     PID;
  static const int     INTERFACE_NUMBER;
  static const uint8_t EP_CTL_OUT;
  static const uint8_t EP_CTL_IN;
  static const uint8_t EP_DATA0_IN;
  static const uint8_t EP_DATA1_IN;

  static const size_t EVENT_DATA_READ_CHUNK;
  static const size_t USB_TIMEOUT;
  static const int    MAX_RETRIES_READ;

  static const uint8_t MODULE_MASTER;
  static const uint8_t MODULE_STATUS;
  static const uint8_t MODULE_VOLTAGE;
  static const uint8_t MODULE_DCTRL;
  static const uint8_t MODULE_DATA0;

  static const uint8_t MASTER_DP23_SOURCE;

private:
  std::shared_ptr<UsbDev> m_usb;
  TBoardConfigRU *        m_config;
  UsbDev::DataBuffer      m_buffer;
  uint32_t                m_readBytes;

  bool m_logging;

  // Triggeroptions
  bool m_enablePulse;
  bool m_enableTrigger;
  int  m_triggerDelay;
  int  m_pulseDelay;

  std::map<uint8_t, std::vector<uint8_t>> m_readoutBuffers;
  std::deque<std::vector<uint8_t>>        m_events;

  // Readout streams
  void fetchEventData();

public:
  // Modules
  std::shared_ptr<TRuDctrlModule>                          dctrl;
  std::shared_ptr<TRuWishboneModule>                       master;
  std::map<uint8_t, std::shared_ptr<TRuTransceiverModule>> transceiver_array;

public:
  TReadoutBoardRU(TBoardConfigRU *config);

  virtual int WriteChipRegister(uint16_t Address, uint16_t Value, TAlpide *chipPtr = 0);
  virtual int ReadRegister(uint16_t Address, uint32_t &Value);
  virtual int WriteRegister(uint16_t Address, uint32_t Value);
  virtual int ReadChipRegister(uint16_t Address, uint16_t &Value, TAlpide *chipPtr = 0);
  virtual int SendOpCode(Alpide::TOpCode OpCode);
  virtual int SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr);
  virtual int SendCommand(Alpide::TCommand OpCode, TAlpide *chipPtr);

  virtual int  SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                int pulseDelay);
  virtual void SetTriggerSource(TTriggerSource triggerSource);
  virtual void StartRun();
  virtual int  Trigger(int nTriggers);
  virtual int  ReadEventData(int &NBytes, unsigned char *Buffer);

  // RU specific functions

  void setDataportSource(uint8_t DP2Source = 255, uint8_t DP3Source = 255);

  // Initialize Readout Unit to start readout with given configuration
  int Initialize();

  void                    registeredWrite(uint16_t module, uint16_t address, uint16_t data);
  void                    registeredRead(uint16_t module, uint16_t address);
  bool                    flush();
  void                    readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer);
  std::vector<ReadResult> readResults();

  void checkGitHash();
  void InitReceivers();
};

#endif // TREADOUTBOARDRU_H

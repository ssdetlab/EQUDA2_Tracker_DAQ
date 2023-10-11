#ifndef READOUTBOARD_H
#define READOUTBOARD_H

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "TAlpide.h"
#include "TBoardConfig.h"

#define MAX_EVENT_SIZE 4096000

class TBoardConfig;

typedef struct {
  int      chipId;
  int      controlInterface;
  int      receiver;
  bool     enabled;
  TAlpide *alpidePtr;
} TChipPos;

//************************************************************
// abstract base class for all readout boards
//************************************************************

class TReadoutBoard {
private:
  int fNChips; // probably obsolete, use fChipPositions.size() instead

protected:
  std::vector<TChipPos>
                fChipPositions; // Antonio : change in protected to access from derived class
  TBoardConfig *fBoardConfig;

  virtual int WriteChipRegister(uint16_t Address, uint16_t Value, TAlpide *chipPtr) = 0;

  int GetControlInterface(uint8_t chipId);
  int GetControlInterface(TAlpide *chipPtr);

  int GetChipById(uint8_t chipId);
  int GetChipById(TAlpide *chipPtr);

  friend class TAlpide; // could be reduced to the relevant methods ReadRegister, WriteRegister

public:
  // TReadoutBoard  (TBoardConfig *config) {};
  TReadoutBoard(TBoardConfig *config);
  ~TReadoutBoard(){};

  int AddChip(uint8_t chipId, int controlInterface, int receiver, TAlpide *chipPtr = 0);

  int GetReceiver(uint8_t chipId);
  int GetReceiver(TAlpide *chipPtr);

  void SetChipEnable(uint8_t chipId, bool Enable);
  void SetChipEnable(TAlpide *chipPtr, bool Enable);
  void SetControlInterface(uint8_t chipId, int controlInterface);
  void SetControlInterface(TAlpide *chipPtr, int controlInterface);
  void SetReceiver(uint8_t chipId, int receiver);
  void SetReceiver(TAlpide *chipPtr, int receiver);

  TBoardConfig *GetConfig() { return fBoardConfig; };

  virtual int ReadRegister(uint16_t Address, uint32_t &Value) = 0;
  virtual int WriteRegister(uint16_t Address, uint32_t Value) = 0;

  virtual int ReadChipRegister(uint16_t Address, uint16_t &Value, TAlpide *chipPtr) = 0;

  // sends op code to all control interfaces
  virtual int SendOpCode(Alpide::TOpCode OpCode) = 0;
  // sends op code to control interface belonging to chip chipId
  virtual int SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr) = 0;

  // sends command code to control interface belonging to chip chipId
  virtual int SendCommand(Alpide::TCommand Command, TAlpide *chipPtr) = 0;

  virtual int  SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                int pulseDelay)               = 0;
  virtual void SetTriggerSource(TTriggerSource triggerSource) = 0;
  virtual void StartRun()                                     = 0;
  virtual int  Trigger(int nTriggers)                         = 0;
  virtual int  ReadEventData(int &          NBytes,
                             unsigned char *Buffer) = 0; // TODO: max buffer size not needed??
};

#endif /* READOUTBOARD_H */

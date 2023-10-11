#ifndef TDATATAKING_H
#define TDATATAKING_H

#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

const int kTrigPerTrain = 100;

typedef struct __TDataTakingParameters : TScanParameters {
  int nTriggers;
} TDataTakingParameters;

class TDataTaking : public TScan {
private:
  int                  m_nTrains;
  int                  m_nLast;
  std::vector<TPixHit> m_stuck;
  TErrorCounter        m_errorCount;
  virtual void         ConfigureChip(TAlpide *chip) = 0;
  void                 ConfigureBoard(TReadoutBoard *board);
  void                 FillHistos(std::vector<TPixHit> *Hits, int board);
  void                 ReadEventData(std::vector<TPixHit> *Hits, int iboard, int nTriggers);
  void                 FindTimeoutHics(int iboard, int *triggerCounts, int nTriggers);

protected:
  THisto CreateHisto();
  void   CalculateTrains();
  int    m_nTriggers;
  bool   m_pulse;
  int    m_pulseLength;
  void   ConfigureFromu(TAlpide *chip);

public:
  TDataTaking(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
              std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
              std::mutex *aMutex);
  ~TDataTaking(){};
  void Init();
  void PrepareStep(int loopIndex)
  {
    if (loopIndex == 0) std::cout << "sending train " << m_value[0] << std::endl;
  };
  void         LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void         LoopEnd(int loopIndex);
  void         Execute();
  virtual void Terminate();
};

#endif

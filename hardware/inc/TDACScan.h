#ifndef TDACSCAN_H
#define TDACSCAN_H

#include "TAlpide.h"
#include "THisto.h"
#include "TScan.h"
#include <mutex>

class TDACScan : public TScan {
private:
  void     ConfigureChip(TAlpide *chip);
  uint16_t m_restoreValue;

protected:
  THisto CreateHisto();
  void   CreateScanParameters() { m_parameters = new TScanParameters; };

public:
  TDACScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
           std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
           std::mutex *aMutex);
  ~TDACScan(){};
  void Init();
  void Execute();
  void Terminate();

  void LoopStart(int loopIndex);
  void LoopEnd(int loopIndex);
  void PrepareStep(int loopIndex);
};

#endif

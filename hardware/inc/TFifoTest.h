#ifndef TFIFOTEST_H
#define TFIFOTEST_H

#include <mutex>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"

typedef struct __TFifoParameters : TScanParameters {
  float voltageScale;
  int   mlvdsStrength;
} TFifoParameters;

class TFifoTest : public TScan {
private:
  TAlpide *m_testChip;
  int      m_boardIndex;

  int  GetChipById(std::vector<TAlpide *> chips, int previousId);
  void ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue, bool &exception);
  void WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue);
  bool TestPattern(int pattern, bool &exception);
  void SetName();

protected:
  THisto CreateHisto();
  void   CreateScanParameters() { m_parameters = new TFifoParameters; };

public:
  TFifoTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
            std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
            std::mutex *aMutex);
  ~TFifoTest(){};
  void Init();
  void Execute();
  void Terminate();
  void LoopEnd(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void PrepareStep(int loopIndex);
  bool IsNominal()
  {
    return ((((TFifoParameters *)m_parameters)->voltageScale > 0.99) &&
            (((TFifoParameters *)m_parameters)->voltageScale < 1.01));
  };
  bool SetParameters(TScanParameters *pars);
  bool IsLower() { return (((TFifoParameters *)m_parameters)->voltageScale < 0.95); };
  bool IsUpper() { return (((TFifoParameters *)m_parameters)->voltageScale > 1.05); };
  int  GetDriver() { return ((TFifoParameters *)m_parameters)->mlvdsStrength; };
};

#endif

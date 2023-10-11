#ifndef TREADOUTTEST_H
#define TREADOUTTEST_H

#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "TDataTaking.h"
#include "THisto.h"
#include "TScan.h"

typedef struct __TReadoutParameters : TDataTakingParameters {
  int   row;
  int   linkSpeed;
  int   occupancy;
  int   driverStrength;
  int   preemp;
  int   pllStages;
  float voltageScale;
} TReadoutParameters;

class TReadoutTest : public TDataTaking {
private:
  void ConfigureChip(TAlpide *chip);
  void ConfigureMask(TAlpide *chip, std::vector<TPixHit> *MaskedPixels);
  void SetName();

protected:
  THisto CreateHisto();
  void   CreateScanParameters() { m_parameters = new TReadoutParameters; };

public:
  TReadoutTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
               std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
               std::mutex *aMutex);
  ~TReadoutTest(){};
  int  GetRow() { return ((TReadoutParameters *)m_parameters)->row; };
  int  GetDriver() { return ((TReadoutParameters *)m_parameters)->driverStrength; };
  int  GetLinkSpeed() { return ((TReadoutParameters *)m_parameters)->linkSpeed; };
  int  GetPreemp() { return ((TReadoutParameters *)m_parameters)->preemp; };
  void Init();
  void PrepareStep(int loopIndex) { (void)(&loopIndex); };
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void Terminate();
  bool SetParameters(TScanParameters *pars);
  void WritePLLReg(const char *fName, THic *aHic);
};

#endif

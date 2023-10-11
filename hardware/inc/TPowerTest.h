#ifndef TPOWERTEST_H
#define TPOWERTEST_H

#include <map>
#include <mutex>
#include <set>
#include <string>

#include "Common.h"
#include "THIC.h"
#include "THisto.h"
#include "TScan.h"

// Test types containing power test
static const std::set<std::string> kPowerTestTypes = {
    "OB HIC Qualification Test",   "IB HIC Qualification Test",   "IB Stave Qualification Test",
    "OB HIC Reception Test",       "OL HS Qualification Test",    "ML HS Qualification Test",
    "OL Stave Qualification Test", "ML Stave Qualification Test", "OL Stave Reception Test",
    "ML Stave Reception Test"};

typedef struct {
  THicType hicType;
  bool     trip;
  bool     tripBB;
  float    iddaSwitchon;
  float    idddSwitchon;
  float    iddaClocked;
  float    idddClocked;
  float    iddaConfigured;
  float    idddConfigured;
  float    ibias0;
  float    ibias3;
  float    rGnd1;
  float    rGnd2;
  float    rDig;
  float    rAna;
  float    maxBias;
  float    ibias[61];
} THicCurrents;

class TPowerTest : public TScan {
private:
  THic * m_testHic;
  void   CreateMeasurements();
  THisto CreateHisto()
  {
    THisto histo;
    return histo;
  };
  void                                DoIVCurve(THicCurrents &result);
  std::map<std::string, THicCurrents> m_hicCurrents;
  bool                                m_ivcurve;

protected:
  void CreateScanParameters() { m_parameters = new TScanParameters; };

public:
  TPowerTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
             std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
             std::mutex *aMutex);
  ~TPowerTest(){};

  void Init();
  void Execute();
  void Terminate();
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void LoopEnd(int loopIndex) { (void)(&loopIndex); };
  void PrepareStep(int loopIndex);
  void MeasureResistances(THic *testHic, THicCurrents &hicCurrents);
  void DigitalCurrentStep(float &dVDig, float &dVAna, float &dIDig, float &dIAna);
  void AnalogCurrentStep(float &dVAna, float &dIAna);
  std::map<std::string, THicCurrents> GetCurrents() { return m_hicCurrents; };
  static bool                         isPerformedDuring(string testType)
  {
    return kPowerTestTypes.find(testType) != kPowerTestTypes.end();
  };
};

#endif

#ifndef TDIGITALSCAN_H
#define TDIGITALSCAN_H

#include <deque>
#include <mutex>
#include <set>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

// Test types containing digital scan
static const std::set<std::string> kDigitalTestTypes = {
    "OB HIC Qualification Test",   "IB HIC Qualification Test",   "IB Stave Qualification Test",
    "OB HIC Reception Test",       "OL HS Qualification Test",    "ML HS Qualification Test",
    "OL Stave Qualification Test", "ML Stave Qualification Test", "OL Stave Reception Test",
    "ML Stave Reception Test"};

typedef struct __TDigitalParameters : TScanParameters {
  float voltageScale;
} TDigitalParameters;

class TDigitalScan : public TMaskScan {
private:
  void ConfigureFromu(TAlpide *chip);
  void FillHistos(std::vector<TPixHit> *Hits, int board);

protected:
  void         ConfigureChip(TAlpide *chip);
  void         ConfigureBoard(TReadoutBoard *board);
  THisto       CreateHisto();
  void         CreateScanParameters() { m_parameters = new TDigitalParameters; };
  virtual void SetName();

public:
  TDigitalScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
               std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
               std::mutex *aMutex);
  virtual ~TDigitalScan(){};

  virtual void Init();
  void         PrepareStep(int loopIndex);
  void         LoopEnd(int loopIndex);
  void         Next(int loopIndex);
  void         LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void         Execute();
  void         Terminate();
  bool         IsNominal()
  {
    return ((((TDigitalParameters *)m_parameters)->voltageScale > 0.99) &&
            (((TDigitalParameters *)m_parameters)->voltageScale < 1.01));
  };
  bool        SetParameters(TScanParameters *pars);
  bool        IsLower() { return (((TDigitalParameters *)m_parameters)->voltageScale < 0.95); };
  bool        IsUpper() { return (((TDigitalParameters *)m_parameters)->voltageScale > 1.05); };
  static bool isPerformedDuring(string testType)
  {
    return kDigitalTestTypes.find(testType) != kDigitalTestTypes.end();
  };
};

class TDigitalWhiteFrame : public TDigitalScan {
protected:
  void SetName();

public:
  TDigitalWhiteFrame(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                     std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                     std::mutex *aMutex);
  virtual ~TDigitalWhiteFrame(){};
  void ConfigureMaskStage(TAlpide *chip, int istage);
  void Init();
};

#endif

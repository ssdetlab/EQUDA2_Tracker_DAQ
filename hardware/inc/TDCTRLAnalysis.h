#ifndef TDCTRLANALYSIS_H
#define TDCTRLANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"


class TDctrlResultChip : public TScanResultChip {
  friend class TDctrlAnalysis;

private:
  bool  slave;
  float m_pos;
  float b_pos;
  float chisq_pos;
  float corr_pos;
  float maxAmp_pos;
  float maxRise_pos;
  float maxFall_pos;
  float m_neg;
  float b_neg;
  float chisq_neg;
  float corr_neg;
  float maxAmp_neg;
  float maxRise_neg;
  float maxFall_neg;

public:
  TDctrlResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TDctrlResultHic : public TScanResultHic {
  friend class TDctrlAnalysis;

private:
  float worst_slope;
  float worst_slopeRatio;
  float worst_maxAmp;
  float worst_chisq;
  float worst_chisqRatio;
  float worst_corr;
  float worst_rise;
  float worst_fall;
  char  m_scanFile[200];

public:
  TDctrlResultHic()
      : TScanResultHic(), worst_slope(1), worst_maxAmp(10), worst_chisq(0), worst_corr(1),
        worst_rise(0), worst_fall(0){};
  void Compare(TScanResultHic *aPrediction);
  void SetScanFile(const char *fName) { strncpy(m_scanFile, fName, sizeof(m_scanFile)); };
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TDctrlResult : public TScanResult {
  friend class TDctrlAnalysis;

public:
  TDctrlResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TDctrlAnalysis : public TScanAnalysis {
private:
  void               WriteResult();
  void               FillVariableList();
  THicClassification GetClassificationIB(TDctrlResultHic *result);
  THicClassification GetClassificationOB(TDctrlResultHic *result);
  bool               ChipIsSlave(common::TChipIndex idx);
  float              Max(float a, float b, float c);
  float              Min(float a, float b, float c);
  void Fit(std::vector<float> x, std::vector<float> y, float &m, float &b, float &corr,
           float &chisq);

protected:
  TScanResultChip *GetChipResult()
  {
    TDctrlResultChip *Result = new TDctrlResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TDctrlResultHic *Result = new TDctrlResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType();
  void   InitCounters();
  void   CalculatePrediction(std::string hicName);

public:
  TDctrlAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TDctrlResult *aResult = 0);
  void Initialize();
  void Finalize();

  std::string GetName() { return "DCTRL Analysis"; }
};

#endif

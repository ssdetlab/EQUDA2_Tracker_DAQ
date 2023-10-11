#ifndef TFASTPOWERANALYSIS_H
#define TFASTPOWERANALYSIS_H

#include "TFastPowerTest.h"
#include "TPowerTest.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TFastPowerResultChip : public TScanResultChip {
public:
  TFastPowerResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp) { (void)fp; };
  float GetVariable(TResultVariable var)
  {
    (void)(&var);
    return 0;
  };
};

class TFastPowerResultHic : public TScanResultHic {
  friend class TFastPowerAnalysis;

private:
  bool  trip;
  bool  tripBB;
  float iddaSwitchon;
  float idddSwitchon;
  float ibias0;
  float ibias3;
  float ibias[50];
  float maxBias;
  char  m_ivFile[200];

protected:
public:
  TFastPowerResultHic() : TScanResultHic(){};
  void SetIVFile(const char *fName) { strcpy(m_ivFile, fName); };
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TFastPowerResult : public TScanResult {
  friend class TFastPowerAnalysis;

private:
protected:
public:
  TFastPowerResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TFastPowerAnalysis : public TScanAnalysis {
private:
  int                m_ivPoints;
  void               CreateIVHisto(TFastPowerResultHic *hicResult);
  void               WriteIVCurve(THic *hic);
  THicClassification GetClassification(THicCurrents currents, TFastPowerResultHic *result);
  //  THicClassification GetClassificationIB(THicCurrents currents);
  THicClassification GetClassificationOB(THicCurrents currents, TFastPowerResultHic *result);

protected:
  TScanResultChip *GetChipResult()
  {
    TFastPowerResultChip *result = new TFastPowerResultChip();
    return result;
  };
  TScanResultHic *GetHicResult()
  {
    TFastPowerResultHic *result = new TFastPowerResultHic();
    return result;
  };
  void   CreateResult(){};
  void   InitCounters(){};
  void   WriteResult();
  void   AnalyseHisto(TScanHisto *histo) { (void)&histo; };
  string GetPreviousTestType();
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TFastPowerAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                     std::vector<THic *> hics, std::mutex *aMutex, TFastPowerResult *aResult = 0);
  void Initialize() { CreateHicResults(); };
  void Run(){};
  void Finalize();

  std::string GetName() { return "Fast Power Analysis"; }
};

#endif

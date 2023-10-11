#ifndef TDIGITALWFANALYSIS_H
#define TDIGITALWFANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TDigitalWFResultChip : public TScanResultChip {
  friend class TDigitalWFAnalysis;

private:
  int                  m_nStuck;
  int                  m_nBadDCol;
  int                  m_nUnmaskable;
  std::vector<TPixHit> m_stuck;
  std::vector<TPixHit> m_stuckOnePerDCol;

public:
  TDigitalWFResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TDigitalWFResultHic : public TScanResultHic {
  friend class TDigitalWFAnalysis;

private:
  float m_backBias;
  int   m_nStuck;
  int   m_nBadDCol;
  int   m_nUnmaskable;
  char  m_stuckFile[200];
  char  m_unmaskedFile[200];
  void  GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TDigitalWFResultHic() : TScanResultHic(){};
  void SetStuckFile(const char *fName) { strcpy(m_stuckFile, fName); };
  void SetUnmaskedFile(const char *fName) { strcpy(m_unmaskedFile, fName); };
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TDigitalWFResult : public TScanResult {
  friend class TDigitalWFAnalysis;

private:
  int m_nTimeout;
  int m_n8b10b;
  int m_nOversize;
  int m_nCorrupt;

public:
  TDigitalWFResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TDigitalWFAnalysis : public TScanAnalysis {
private:
  int  m_ninj;
  void InitCounters();
  void FillVariableList();
  //  void WriteHitData     (TScanHisto *histo, int row);
  void                 WriteResult();
  void                 WriteStuckPixels(THic *hic);
  void                 WriteUnmaskedPixels(THic *hic);
  void                 WritePixels(THic *hic, std::vector<TPixHit> pixels, const char *fName);
  THicClassification   GetClassificationOB(TDigitalWFResultHic *result);
  THicClassification   GetClassificationIB(TDigitalWFResultHic *result);
  std::vector<TPixHit> m_unmaskable;

protected:
  TScanResultChip *GetChipResult()
  {
    TDigitalWFResultChip *Result = new TDigitalWFResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TDigitalWFResultHic *Result = new TDigitalWFResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType() { return string(""); }; // done only once
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TDigitalWFAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                     std::vector<THic *> hics, std::mutex *aMutex, TDigitalWFResult *aResult = 0);

  void Initialize();
  void Finalize();

  std::string GetName() { return "Digital WF Analysis"; }
};

#endif

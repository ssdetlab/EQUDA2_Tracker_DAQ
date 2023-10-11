#ifndef TREADOUTANALYSIS_H
#define TREADOUTANALYSIS_H

#include <deque>
#include <map>
#include <mutex>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TReadoutResultChip : public TScanResultChip {
  friend class TReadoutAnalysis;

private:
  int m_missingHits;
  int m_deadPixels;
  int m_ineffPixels;
  int m_extraHits;
  int m_noisyPixels;

public:
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
  TReadoutResultChip() : TScanResultChip(){};
};

class TReadoutResultHic : public TScanResultHic {
  friend class TReadoutAnalysis;

private:
  TErrorCounter m_errorCounter;
  int           m_linkSpeed;
  int           m_driver;
  int           m_preemp;
  int           m_missingHits;
  int           m_deadPixels;
  int           m_ineffPixels;
  int           m_extraHits;
  int           m_noisyPixels;
  void          GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TReadoutResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TReadoutResult : public TScanResult {
  friend class TReadoutAnalysis;

private:
public:
  TReadoutResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TReadoutAnalysis : public TScanAnalysis {
private:
  int                m_nTrig;
  int                m_occ;
  int                m_row;
  void               FillVariableList(){};
  bool               IsInjected(int col, int row);
  void               WriteResult();
  THicClassification GetClassificationOB(TReadoutResultHic *result);
  THicClassification GetClassificationIB(TReadoutResultHic *result);

protected:
  TScanResultChip *GetChipResult()
  {
    TReadoutResultChip *Result = new TReadoutResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TReadoutResultHic *Result = new TReadoutResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  void   InitCounters();
  string GetPreviousTestType() { return string(""); }; // done only once ?
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TReadoutAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                   std::vector<THic *> hics, std::mutex *aMutex, TReadoutResult *aResult = 0);
  void Initialize();
  void Finalize();

  std::string GetName() { return "Readout Analysis"; }
};

#endif

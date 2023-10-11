#ifndef TNOISEANALYSIS_H
#define TNOISEANALYSIS_H

#include <deque>
#include <map>
#include <mutex>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TNoiseResultChip : public TScanResultChip {
  friend class TNoiseAnalysis;
  friend class TApplyMask;

private:
  std::vector<TPixHit> m_noisyPixels;
  double               m_occ;

public:
  TNoiseResultChip() : TScanResultChip(){};
  void                 AddNoisyPixel(TPixHit pixel) { m_noisyPixels.push_back(pixel); };
  void                 SetOccupancy(double occ) { m_occ = occ; };
  void                 WriteToFile(FILE *fp);
  float                GetVariable(TResultVariable var);
  std::vector<TPixHit> GetNoisyPixels() { return m_noisyPixels; };
};

class TNoiseResultHic : public TScanResultHic {
  friend class TNoiseAnalysis;
  friend class TApplyMask;

private:
  bool          m_isMasked;
  double        m_occ;
  double        m_maxChipOcc;
  int           m_nNoisy;
  float         m_backBias;
  char          m_noisyFile[200];
  TErrorCounter m_errorCounter;
  void          GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TNoiseResultHic() : TScanResultHic(){};
  void SetNoisyFile(const char *fName) { strcpy(m_noisyFile, fName); };
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TNoiseResult : public TScanResult {
  friend class TNoiseAnalysis;
  friend class TApplyMask;

private:
  std::vector<TPixHit> m_noisyPixels;

public:
  TNoiseResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TNoiseAnalysis : public TScanAnalysis {
private:
  int   m_nTrig;
  float m_noiseCut;
  bool  m_isMasked;
  void  WriteResult();
  void  FillVariableList();
  void  WriteNoisyPixels(THic *hic);

protected:
  TScanResultChip *GetChipResult()
  {
    TNoiseResultChip *Result = new TNoiseResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TNoiseResultHic *Result = new TNoiseResultHic();
    return Result;
  };
  void               CreateResult(){};
  void               AnalyseHisto(TScanHisto *histo);
  void               InitCounters();
  string             GetPreviousTestType();
  void               CalculatePrediction(std::string hicName) { (void)hicName; };
  THicClassification GetClassification(TNoiseResultHic *result, THic *hic);

public:
  TNoiseAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TNoiseResult *aResult = 0);
  void Initialize();
  void Finalize();

  std::string GetName() { return "Noise Occupancy"; }
};

#endif

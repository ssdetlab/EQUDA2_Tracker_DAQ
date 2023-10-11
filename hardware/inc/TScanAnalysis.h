#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <map>
#include <mutex>
#include <string.h>
#include <string>

#include "Common.h"

using namespace std;

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;
class THic;
struct TScanParameters__;
typedef TScanParameters__ TScanParameters;

// Warning: has to stay ordered such, that worse test result = higher entry
typedef enum hicClassification {
  CLASS_UNTESTED,
  //  CLASS_GREEN,
  //  CLASS_ORANGE,
  CLASS_GOLD,
  CLASS_SILVER,
  CLASS_BRONZE,
  CLASS_GOLD_NOBB,   // used only for power test
  CLASS_SILVER_NOBB, // used only for power test
  CLASS_BRONZE_NOBB, // used only for power test
  CLASS_PARTIAL,
  CLASS_PARTIALB,
  CLASS_NOBB,
  CLASS_NOBBB,
  CLASS_RED,
  CLASS_ABORTED
} THicClassification;

typedef enum {
  status,
  deadPix,
  noisyPix,
  ineffPix,
  stuckPix,
  unmaskablePix,
  badDcol,
  thresh,
  noise,
  noThreshPix,
  hotPix,
  threshRms,
  noiseRms,
  vcasn,
  ithr,
  noiseOcc,
  Err0,
  Errf,
  Erra,
  Err5,
  ErrBusyOn,
  ErrBusyOff
} TResultVariable;

THicClassification Worst(THicClassification a, THicClassification b);

// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
protected:
  string m_outputPath;

public:
  TScanResultChip(){};
  virtual void  WriteToFile(FILE *fp)            = 0;
  virtual float GetVariable(TResultVariable var) = 0;
  void          SetOutputPath(string path) { m_outputPath = path; };
  string        GetOutputPath() { return m_outputPath; };
};

class TScanResultHic {
  friend class TScanAnalysis;

protected:
  std::map<int, TScanResultChip *> m_chipResults;
  std::string                      m_hicName;
  int                              m_modId;
  char                             m_resultFile[300];
  THicClassification               m_class;

  string                   m_outputPath;
  string                   m_pdfPath;
  TScanParameters *        m_scanParameters;
  bool                     m_valid;  // used for predictions only
  bool                     m_hasPDF; // true if PDF was created for I-V plot
  virtual void             Compare(TScanResultHic *aPrediction) { (void)aPrediction; };
  std::vector<std::string> m_cuts;
  void                     ClearCuts() { m_cuts.clear(); };
  void                     AddCut(std::string aCut) { m_cuts.push_back(aCut); };

public:
  TScanResultHic()
  {
    m_valid = false;
    ClearCuts();
  };
  const char * WriteHicClassification();
  void         SetValidity(bool valid) { m_valid = valid; };
  bool         IsValid() { return m_valid; };
  virtual void WriteToFile(FILE *fp) = 0;
  // virtual void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
  // virtual void WriteClassToDB(AlpideDB *db, ActivityDB::activity &activity, std::string scanName);
  int          AddChipResult(int aChipId, TScanResultChip *aChipResult);
  void SetResultFile(const char *fName) { strncpy(m_resultFile, fName, sizeof(m_resultFile)); };
  THicClassification GetClassification() { return m_class; };
  void               SetClassification(THicClassification aClass) { m_class = aClass; };
  std::map<int, TScanResultChip *> DeleteThisToo() { return m_chipResults; };
  float                            GetVariable(int chip, TResultVariable var);
  string                           GetOutputPath() { return m_outputPath; };
  string                           GetParameterFile();
  std::vector<std::string>         GetCuts() { return m_cuts; };
  std::string                      GetName() { return m_hicName; };
  int                              GetModId() { return m_modId; }
  void                             SetHasPDF(const bool pdf) { m_hasPDF = pdf; };
  void                             SetPDFPath(const string path) { m_pdfPath = path; };
  bool                             HasPDF() { return m_hasPDF; };
  string                           GetPDFPath() { return m_hasPDF ? m_pdfPath : 0; };
};

// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
private:
protected:
  std::map<int, TScanResultChip *>        m_chipResults;
  std::map<std::string, TScanResultHic *> m_hicResults;

public:
  TScanResult(){};
  virtual ~TScanResult(){};
  // virtual TScanResult *clone() const=0;
  // TScanResult(const TScanResult &other){m_chipResults=other.m_chipResults;}
  // assignment operation from my base class
  // TScanResult &operator=(const TScanResult &other){if (&other!=this) return *this;
  // m_chipResults=other.m_chipResults; return *this;}
  int              AddChipResult(common::TChipIndex idx, TScanResultChip *aChipResult);
  int              AddChipResult(int aIntIndex, TScanResultChip *aChipResult);
  int              AddHicResult(std::string hicId, TScanResultHic *aHicResult);
  int              GetNChips() { return (int)m_chipResults.size(); };
  int              GetNHics() { return (int)m_hicResults.size(); };
  void             WriteToFile(const char *fName);
  void             ForceClassification(THicClassification aClass);
  virtual void     WriteToFileGlobal(FILE *fp) = 0;
  // void             WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
  TScanResultChip *GetChipResult(common::TChipIndex idx);
  TScanResultHic * GetHicResult(std::string hic);
  std::map<std::string, TScanResultHic *> *GetHicResults() { return &m_hicResults; };
};

class TScanAnalysis {
protected:
  std::deque<TScanHisto> *                m_histoQue;
  std::vector<common::TChipIndex>         m_chipList;
  std::vector<THic *>                     m_hics;
  std::mutex *                            m_mutex;
  std::map<const char *, TResultVariable> m_variableList;
  TScan *                                 m_scan;
  TScanConfig *                           m_config;
  TScanResult *                           m_result;
  TScanResult *                           m_prediction;
  bool                                    m_first;
  bool                                    m_started;
  bool                                    m_finished;
  virtual TScanResultChip *               GetChipResult() = 0;
  virtual TScanResultHic *                GetHicResult()  = 0;
  void                                    CreateHicResults();
  void                                    CreatePrediction();
  void                                    ComparePrediction(std::string hicName);
  virtual void CalculatePrediction(std::string hicName) = 0; // { (void)hicName; };
  virtual void CreateResult()                           = 0;
  int          ReadChipList();
  virtual void AnalyseHisto(TScanHisto *histo) = 0;
  virtual void InitCounters()                  = 0;
  int          GetPreviousActivityType();
  // bool         GetPreviousActivity(string compName, ActivityDB::activityLong &act);
  // bool FillPreviousActivities(string hicName, std::vector<ActivityDB::activityLong> *activities);
  // bool GetPreviousParamValue(string hicTestName, string chipTestName, ActivityDB::activityLong &act,
                            //  float &value);
  int  GetChildList(int id, std::string hicName, std::vector<std::string> &childrenNames);
  int  GetPreviousComponentType(std::string prevTestType);
  int  GetComponentType();
  TScanResultHic *FindHicResultForChip(common::TChipIndex chip);
  virtual string  GetPreviousTestType() = 0;
  void DoCut(THicClassification &hicClass, THicClassification failClass, int value, string cutName,
             TScanResultHic *result, bool minCut = false, int chipId = -1);
  void WriteCut(string cutText, TScanResultHic *result);

public:
  TScanAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                std::vector<THic *> hics, std::mutex *aMutex);
  virtual ~TScanAnalysis(){};
  virtual void                            Initialize() = 0;
  virtual void                            Run();
  virtual void                            Finalize() = 0;
  virtual std::string                     GetName() { return typeid(*this).name(); }
  bool                                    IsFinished() const { return m_finished; }
  std::map<const char *, TResultVariable> GetVariableList() { return m_variableList; }
  float              GetVariable(std::string aHic, int chip, TResultVariable var);
  static const char *WriteHicClassification(THicClassification hicClass);
  void               WriteHicClassToFile(std::string hicName);
  THicClassification GetScanClassification();
  void               PrintHicClassification(TScanResultHic *res);
};

#endif

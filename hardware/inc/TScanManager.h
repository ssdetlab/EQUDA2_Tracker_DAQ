#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanFactory.h"

class TScanManager {
public:
  TScanManager();

  TScanFactory::TScanObjects AddScan(TScanType scanType, TScanResult *scanResult = nullptr);
  void                       Init();
  void                       Reset();
  void                       Run();
  void                       PrintClassifications();
  void                       UpdateClassifications();

  static void Scan(TScan *scan);
  static void Analysis(TScanAnalysis *analysis);

protected:
  TConfig *                    fConfig;
  std::vector<TReadoutBoard *> fBoards;
  TBoardType                   boardType;
  std::vector<TAlpide *>       fChips;
  std::vector<THic *>          fHICs;
  std::deque<TScanHisto>       fHistoQue;
  std::mutex                   fMutex;
  // std::vector<TScan*>          fScans;
  // std::vector<TScanAnalysis*>  fAnalyses;
  // std::vector<TScanParameters*> fScanParameters;
  // std::vector<TScanResult*>    fResults;
  // std::vector<TScanType>       fScanTypes;

  std::vector<TScanFactory::TScanObjects> fScanObjects;

  static std::string GetResult(THicClassification cl);
};

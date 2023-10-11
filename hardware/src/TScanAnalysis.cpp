#include <iostream>
#include <unistd.h>

// #include "DBHelpers.h"
#include "TScanAnalysis.h"

#include "THIC.h"
#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

THicClassification Worst(THicClassification a, THicClassification b)
{
  if (a > b)
    return a;
  else
    return b;
}

TScanAnalysis::TScanAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aConfig,
                             std::vector<THic *> hics, std::mutex *aMutex)
{
  m_histoQue = histoQue;
  m_mutex    = aMutex;
  m_scan     = aScan;
  m_config   = aConfig;
  m_hics     = hics;
  m_first    = true;
  m_started  = false;
  m_finished = false;
  m_chipList.clear();
}

int TScanAnalysis::GetPreviousActivityType()
{
  // return DbGetActivityTypeId(m_config->GetDatabase(), GetPreviousTestType());
}


int TScanAnalysis::GetChildList(int id, string hicName, std::vector<std::string> &childrenNames)
{
  // std::vector<TChild> children;
  // childrenNames.clear();
  // int prevCompType = GetPreviousComponentType(GetPreviousTestType());
  // int compType     = GetComponentType();
  // if (prevCompType < 0) return 0;
  // if (prevCompType == compType) { // look at component itself, not at children
    // childrenNames.push_back(hicName);
    // return 1;
  // }
  // DbGetListOfChildren(m_config->GetDatabase(), id, children);

  // for (unsigned int i = 0; i < children.size(); i++) {
    // if (children.at(i).Type == prevCompType) childrenNames.push_back(children.at(i).Name);
  // }
  // return childrenNames.size();
}

// TODO: Include OB Half-Stave test, here and in all GetPreviousTestTypes
int TScanAnalysis::GetPreviousComponentType(std::string prevTestType)
{
  // if (prevTestType == "ALPIDEB Chip Testing Analysis") {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "ALPIDEB Chip");
  // }
  // else if ((prevTestType == "OB HIC Qualification Test") ||
          //  (prevTestType == "OB HIC Endurance Test") || (prevTestType == "OB HIC Reception Test") ||
          //  (prevTestType == "OL HS Qualification Test") ||
          //  (prevTestType == "ML HS Qualification Test") ||
          //  (prevTestType == "ML Stave Qualification Test") ||
          //  (prevTestType == "OL Stave Qualification Test") ||
          //  (prevTestType == "ML Stave Reception Test") ||
          //  (prevTestType == "OL Stave Reception Test")) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "Outer Barrel HIC Module");
  // }

  // else if ((prevTestType == "IB Stave Qualification Test")) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "IB Stave");
  // }
  // else if ((prevTestType == "IB HIC Qualification Test")) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "Inner Barrel HIC Module");
  // }
  return -1;
}


// component type of current test
// check (IB Stave?)
int TScanAnalysis::GetComponentType()
{
return 1;
  // if (!(m_config->GetDatabase())) return -1;
  // if ((m_config->GetTestType() == OBQualification) || (m_config->GetTestType() == OBEndurance) ||
      // (m_config->GetTestType() == OBReception) || (m_config->GetTestType() == OBPower) ||
      // (m_config->GetTestType() == OBHalfStaveOL) || (m_config->GetTestType() == OBHalfStaveML) ||
      // (m_config->GetTestType() == OBStaveOL) || (m_config->GetTestType() == OBStaveML) ||
      // (m_config->GetTestType() == StaveReceptionOL) ||
      // (m_config->GetTestType() == StaveReceptionML)) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "Outer Barrel HIC Module");
  // }
  // else if ((m_config->GetTestType() == IBQualification) ||
          //  (m_config->GetTestType() == IBEndurance)) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "Inner Barrel HIC Module");
  // }
  // else if ((m_config->GetTestType() == IBStave)) {
    // return DbGetComponentTypeId(m_config->GetDatabase(), "IB Stave");
  // }
  // else
    // return -1;
}


// bool TScanAnalysis::GetPreviousActivity(string compName, ActivityDB::activityLong &act)
// {
  // return DbGetLatestActivity(m_config->GetDatabase(), GetPreviousActivityType(), compName, act);
// }


// bool TScanAnalysis::FillPreviousActivities(string                                 hicName,
                                          //  std::vector<ActivityDB::activityLong> *activities)
// {
  // std::vector<std::string> childNames;
  // int                      compType = GetComponentType();
  // if (!(m_config->GetDatabase())) return false;
  // int compId = DbGetComponentId(m_config->GetDatabase(), m_config->GetDatabase()->GetProjectId(),
                                // compType, hicName);
  // GetChildList(compId, hicName, childNames);

  // std::cout << "Number of children = " << childNames.size() << std::endl;
  // for (unsigned int i = 0; i < childNames.size(); i++) {
    // ActivityDB::activityLong act;
    // if (GetPreviousActivity(childNames.at(i), act)) {
      // activities->push_back(act);
    // }
  // }

  // if (activities->size() == 0) {
    // std::cout << "No previous activities found " << std::endl;
    // //    prediction->SetValidity(false);
    // return false;
  // }
  // return true;
// }


// bool TScanAnalysis::GetPreviousParamValue(string hicTestName, string chipTestName,
                                          // ActivityDB::activityLong &act, float &value)
// {
  // if (GetPreviousTestType() == "ALPIDEB Chip Testing Analysis") {
    // return DbFindParamValue(act.Parameters, chipTestName, value);
  // }
  // else {
    // return DbFindParamValue(act.Parameters, hicTestName, value);
  // }
// }


int TScanAnalysis::ReadChipList()
{
  m_chipList = m_scan->GetChipList();
  return m_chipList.size();
}


// Create prediction as hic results
// TODO: do we need chip results? (probably not)
// TODO: validity flag? this would require a new structure
void TScanAnalysis::CreatePrediction()
{
  if (m_hics.size() == 0) {
    std::cout << "Warning (TScanAnalysis::CreateResult): hic list is empty, doing nothing"
              << std::endl;
    return;
  }
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    TScanResultHic *hicResult = GetHicResult();
    m_prediction->AddHicResult(m_hics.at(i)->GetDbId(), hicResult);
  }
}


void TScanAnalysis::ComparePrediction(std::string hicName)
{
  TScanResultHic *prediction;
  TScanResultHic *result;

  try {
    prediction = m_prediction->GetHicResult(hicName);
    result     = m_result->GetHicResult(hicName);
  }
  catch (...) {
    std::cout << "Error: prediction or result not found for hic " << hicName << std::endl;
    return;
  }

  if ((!prediction->IsValid()) || (!result->IsValid())) {
    std::cout << "Error: prediction or result not valid for hic " << hicName << std::endl;
    return;
  }

  result->Compare(prediction);
}


void TScanAnalysis::CreateHicResults()
{
  if (m_hics.size() == 0) {
    std::cout << "Warning (TScanAnalysis::CreateResult): hic list is empty, doing nothing"
              << std::endl;
    return;
  }

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TScanResultChip *  chipResult = GetChipResult();
    common::TChipIndex idx        = m_chipList.at(i);
    m_result->AddChipResult(idx, chipResult);
  }

  for (unsigned int i = 0; i < m_hics.size(); i++) {
    TScanResultHic *hicResult   = GetHicResult();
    hicResult->m_class          = CLASS_UNTESTED;
    hicResult->m_hicName        = m_hics.at(i)->GetDbId();
    hicResult->m_modId          = m_hics.at(i)->GetModId();
    hicResult->m_outputPath     = m_config->GetDataPath(m_hics.at(i)->GetDbId());
    hicResult->m_scanParameters = m_scan->GetParameters();
    for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip++) {
      if (m_hics.at(i)->ContainsChip(m_chipList.at(iChip))) {
        m_result->GetChipResult(m_chipList.at(iChip))->SetOutputPath(hicResult->GetOutputPath());
        hicResult->AddChipResult(m_chipList.at(iChip).chipId & 0xf,
                                 m_result->GetChipResult(m_chipList.at(iChip)));
      }
    }
    hicResult->m_hasPDF = false;
    m_result->AddHicResult(m_hics.at(i)->GetDbId(), hicResult);
  }
}


TScanResultHic *TScanAnalysis::FindHicResultForChip(common::TChipIndex chip)
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    if (m_hics.at(i)->ContainsChip(chip)) {
      return m_result->GetHicResult(m_hics.at(i)->GetDbId());
    }
  }
  return 0;
}


void TScanAnalysis::Run()
{
  m_started = true;

  while ((!fScanAbort) && (!fScanAbortAll) && (m_histoQue->size() == 0)) {
    sleep(1);
  }

  while ((!fScanAbort) && (!fScanAbortAll) && ((m_scan->IsRunning() || (m_histoQue->size() > 0)))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()))
        ;

      TScanHisto histo = std::move(m_histoQue->front());
      m_histoQue->pop_front();
      m_mutex->unlock();

      if (m_first) {
        // histo.GetChipList(m_chipList);
        InitCounters();
        m_first = false;
      }

      AnalyseHisto(&histo);
    }
    else
      usleep(300);
  }
}

float TScanAnalysis::GetVariable(std::string aHic, int chip, TResultVariable var)
{
  if (!m_finished) {
    std::cout << "Error: analysis not finished yet" << std::endl;
    return 0;
  }
  return m_result->GetHicResult(aHic)->GetVariable(chip, var);
}

// returns the classification of the scan
// this is defined as the classification of the worst HIC
THicClassification TScanAnalysis::GetScanClassification()
{
  THicClassification                                result     = CLASS_UNTESTED;
  std::map<std::string, TScanResultHic *> *         hicResults = m_result->GetHicResults();
  std::map<std::string, TScanResultHic *>::iterator it;
  for (it = hicResults->begin(); it != hicResults->end(); it++) {
    if (!it->second->IsValid()) continue;
    if (it->second->GetClassification() > result) result = it->second->GetClassification();
  }
  return result;
}

const char *TScanAnalysis::WriteHicClassification(THicClassification hicClass)
{
  if (hicClass == CLASS_UNTESTED)
    return "Untested";
  else if (hicClass == CLASS_GOLD)
    return "Gold";
  else if (hicClass == CLASS_SILVER)
    return "Silver";
  else if (hicClass == CLASS_BRONZE)
    return "Bronze";
  else if (hicClass == CLASS_GOLD_NOBB)
    return "Gold, no back bias";
  else if (hicClass == CLASS_SILVER_NOBB)
    return "Silver, no back bias";
  else if (hicClass == CLASS_BRONZE_NOBB)
    return "Bronze, no back bias";
  else if (hicClass == CLASS_RED)
    return "Not working";
  else if (hicClass == CLASS_ABORTED)
    return "Scan aborted";
  else
    return "Unknown";
}

void TScanAnalysis::PrintHicClassification(TScanResultHic *res)
{
  std::cout << "==> " << GetName() << " - Classification " << res->GetName() << " (M"
            << res->GetModId() << "): " << WriteHicClassification(res->GetClassification())
            << std::endl;
}

void TScanAnalysis::WriteHicClassToFile(std::string hicName)
{
  char            fName[300];
  TScanResultHic *hicResult = m_result->GetHicResult(hicName);

  if (hicResult) {
    std::string writeLine = std::string(m_scan->GetName()) + std::string(": ") +
                            std::string(hicResult->WriteHicClassification());
    sprintf(fName, "%s/Classification.dat", hicResult->GetOutputPath().c_str());
    FILE *fp = fopen(fName, "a");
    fprintf(fp, "%s\n", writeLine.c_str());
    fclose(fp);
    std::cout << writeLine << std::endl;
  }
}


void TScanAnalysis::WriteCut(string cutText, TScanResultHic *result)
{
  char fName[300];
  sprintf(fName, "%s/FailedCuts.txt", result->GetOutputPath().c_str());
  FILE * fp   = fopen(fName, "a");
  string text = string(m_scan->GetName()) + ": " + cutText;
  fprintf(fp, "%s\n", text.c_str());
  fclose(fp);
}


// DoCut checks a variable against a cut and sets the classification accordingly
// in case of failure an output is printed to the terminal
// hicClass: has to contain the current hic classification, is modified in case of failure
// failClass: the classification that is assigned in case of failure
// value: the value of the variable to be tested
// cutName: the name of the cut, as defined in TScanConfig
// minCut: if true, value is required to be >= the cut value, otherwise <=
void TScanAnalysis::DoCut(THicClassification &hicClass, THicClassification failClass, int value,
                          string cutName, TScanResultHic *result, bool minCut, int chipId)
{
  bool failed = false;
  int  cut    = m_config->GetParamValue(cutName);
  if (minCut) {
    if (value < cut) failed = true;
  }
  else {
    if (value > cut) failed = true;
  }
  // the message is printed for every failed cut;
  // however the classification is changed only if the new classification is worse than the previous
  // one
  if (failed) {
    std::string cutText;
    if (chipId < 0) {
      cutText = string("Hic failed ") + string(WriteHicClassification(failClass)) +
                string(" cut ") + cutName + string(": cut = ") + to_string(cut) +
                string(", value = ") + to_string(value);
    }
    else {
      cutText = string("Chip Id ") + to_string(chipId) + string(" failed ") +
                string(WriteHicClassification(failClass)) + string(" cut ") + cutName +
                string(": cut = ") + to_string(cut) + string(", value = ") + to_string(value);
    }
    std::cout << cutText << std::endl;
    result->AddCut(cutText);
    WriteCut(cutText, result);
    if (failClass > hicClass) hicClass = failClass;
  }
}


int TScanResult::AddChipResult(common::TChipIndex idx, TScanResultChip *aChipResult)
{
  int id = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf);
  m_chipResults.insert(std::pair<int, TScanResultChip *>(id, aChipResult));
  return m_chipResults.size();
}

int TScanResult::AddChipResult(int aIntIndex, TScanResultChip *aChipResult)
{
  m_chipResults.insert(std::pair<int, TScanResultChip *>(aIntIndex, aChipResult));

  return m_chipResults.size();
}

int TScanResult::AddHicResult(std::string hicId, TScanResultHic *aHicResult)
{
  m_hicResults.insert(std::pair<std::string, TScanResultHic *>(hicId, aHicResult));

  return m_hicResults.size();
}

int TScanResultHic::AddChipResult(int aChipId, TScanResultChip *aChipResult)
{
  m_chipResults.insert(std::pair<int, TScanResultChip *>(aChipId, aChipResult));

  return m_chipResults.size();
}

const char *TScanResultHic::WriteHicClassification()
{
  return TScanAnalysis::WriteHicClassification(m_class);
}

float TScanResultHic::GetVariable(int chip, TResultVariable var)
{
  std::map<int, TScanResultChip *>::iterator it;
  it = m_chipResults.find(chip);
  if (it != m_chipResults.end()) {
    return it->second->GetVariable(var);
  }
  else {
    std::cout << "Error, bad chip ID" << std::endl;
    return 0;
  }
}

// void TScanResultHic::WriteClassToDB(AlpideDB *db, ActivityDB::activity &activity,
                                    // std::string scanName)
// {
  // DbAddParameter(db, activity, string("Classification ") + scanName, (float)m_class,
                //  GetParameterFile());
// }


// void TScanResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), string(m_resultFile));
// }


std::string TScanResultHic::GetParameterFile()
{
  return m_outputPath + std::string("/DBParameters.dat");
}


void TScanResult::ForceClassification(THicClassification aClass)
{
  std::cout << "Warning: forcing all Hic results to "
            << TScanAnalysis::WriteHicClassification(aClass) << std::endl;
  for (std::map<std::string, TScanResultHic *>::iterator it = m_hicResults.begin();
       it != m_hicResults.end(); ++it) {
    it->second->SetClassification(aClass);
  }
}

TScanResultChip *TScanResult::GetChipResult(common::TChipIndex idx)
{
  for (std::map<int, TScanResultChip *>::iterator it = m_chipResults.begin();
       it != m_chipResults.end(); ++it) {
    int id = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf);
    if (it->first == id) return it->second;
  }
  return 0;
}

TScanResultHic *TScanResult::GetHicResult(std::string hic)
{
  std::map<std::string, TScanResultHic *>::iterator it;
  it = m_hicResults.find(hic);
  if (it != m_hicResults.end()) return it->second;
  return 0;
}

void TScanResult::WriteToFile(const char *fName)
{
  FILE *fp = fopen(fName, "a");

  fprintf(fp, "Number of chips: %d\n", (int)m_chipResults.size());

  WriteToFileGlobal(fp);

  for (std::map<int, TScanResultChip *>::iterator it = m_chipResults.begin();
       it != m_chipResults.end(); ++it) {
    int idx = it->first;
    fprintf(fp, "\nBoard %d, Receiver %d, Chip %d:\n", (idx >> 8) & 0xf, (idx >> 4) & 0xf,
            idx & 0xf);
    it->second->WriteToFile(fp);
  }

  fclose(fp);
}

// void TScanResult::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::map<std::string, TScanResultHic *>::iterator it;
  // for (it = m_hicResults.begin(); it != m_hicResults.end(); it++) {
    // if (!it->second->IsValid()) continue;
    // it->second->WriteToDB(db, activity);
  // }
// }

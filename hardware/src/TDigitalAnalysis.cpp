#include "TDigitalAnalysis.h"
// #include "DBHelpers.h"
#include "TDigitalScan.h"
#include <iostream>
#include <string>
#include <vector>

TDigitalAnalysis::TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                   TScanConfig *aScanConfig, std::vector<THic *> hics,
                                   std::mutex *aMutex, TDigitalResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  m_ninj = m_config->GetParamValue("NINJ");
  if (aResult)
    m_result = aResult;
  else
    m_result = new TDigitalResult();

  m_prediction = new TDigitalResult();
  FillVariableList();
}

// TODO: Implement HasData
bool TDigitalAnalysis::HasData(TScanHisto &histo, common::TChipIndex idx, int col) { return true; }

void TDigitalAnalysis::FillVariableList()
{
  // m_variableList.insert (std::pair <const char *, TResultVariable> ("Chip Status", status));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# of dead Pixels", deadPix));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# of noisy Pixels", noisyPix));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# of ineff Pixels", ineffPix));
  // m_variableList.insert (std::pair <const char *, TResultVariable> ("# of bad double columns",
  // badDcol));
}

string TDigitalAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBQualification:
    if ((m_scan->GetParameters())->backBias == 0)
      return string("ALPIDEB Chip Testing Analysis");
    else
      return ("");
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Qualification Test");
  case OBHalfStaveOL:
    return string("OB HIC Reception Test");
  case OBHalfStaveML:
    return string("OB HIC Reception Test");
  case IBQualification:
    if ((m_scan->GetParameters())->backBias == 0)
      return string("ALPIDEB Chip Testing Analysis");
    else
      return ("");
  case IBEndurance:
    return string("IB HIC Qualification Test");
  case IBStave:
    return string("IB HIC Qualification Test");
  case IBStaveEndurance:
    return string("IB Stave Qualification Test");
  case OBStaveOL:
    return string("OL HS Qualification Test");
  case OBStaveML:
    return string("ML HS Qualification Test");
  case StaveReceptionOL:
    return string("OL Stave Qualification Test");
  case StaveReceptionML:
    return string("ML Stave Qualification Test");
  default:
    return string("");
  }
}

void TDigitalAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
  CreatePrediction();
  TTestType testType;
  testType = m_config->GetTestType();
  if (testType != OBHalfStaveOLFAST && testType != OBHalfStaveMLFAST && testType != OBStaveOLFAST &&
      testType != OBStaveMLFAST) {
//     for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
//       CalculatePrediction(m_hics.at(ihic)->GetDbId());
//     }
  }
}

void TDigitalAnalysis::InitCounters()
{
  m_counters.clear();
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TDigitalCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.nCorrect   = 0;
    counter.nIneff     = 0;
    counter.nNoisy     = 0;
    m_counters.push_back(counter);
  }

  std::map<std::string, TScanResultHic *>::iterator it;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TDigitalResultHic *result = (TDigitalResultHic *)it->second;
    result->m_backBias        = m_scan->GetBackBias();
    result->m_lower           = ((TDigitalScan *)m_scan)->IsLower();
    result->m_upper           = ((TDigitalScan *)m_scan)->IsUpper();
    result->m_nominal         = ((TDigitalScan *)m_scan)->IsNominal();
  }
}


void TDigitalAnalysis::CalculatePrediction(std::string hicName)
{
  // std::vector<ActivityDB::activityLong> activities;
  // TDigitalResultHic *                   prediction;
  // try {
    // prediction = (TDigitalResultHic *)m_prediction->GetHicResult(hicName);
  // }
  // catch (...) {
    // std::cout << "Error: prediction not found for hic " << hicName << std::endl;
    // return;
  // }

// //   prediction->SetValidity(FillPreviousActivities(hicName, &activities));
// //   if (!prediction->IsValid()) return;

  // // do the calculation here
  // for (unsigned int i = 0; i < activities.size(); i++) {
    // float value;
    // bool  found = false;
    // if (m_scan->GetBackBias() == 0) {
      // if (((TDigitalScan *)m_scan)->IsNominal())
        // found = GetPreviousParamValue("Bad pixels digital (nominal)", "Dead Pixels",
                                      // activities.at(i), value);
      // else if (((TDigitalScan *)m_scan)->IsLower())
        // found = GetPreviousParamValue("Bad pixels digital (lower)", "Dead Pixels", activities.at(i),
                                      // value);
      // else if (((TDigitalScan *)m_scan)->IsUpper())
        // found = GetPreviousParamValue("Bad pixels digital (upper)", "Dead Pixels", activities.at(i),
                                      // value);
    // }
    // else if (fabs(m_scan->GetBackBias() - 3) < 0.01) {
      // if (((TDigitalScan *)m_scan)->IsNominal())
        // found = GetPreviousParamValue("Bad pixels digital (nominal) BB 3V", "Dead Pixels",
                                      // activities.at(i), value);
      // else if (((TDigitalScan *)m_scan)->IsLower())
        // found = GetPreviousParamValue("Bad pixels digital (lower) BB 3V", "Dead Pixels",
                                      // activities.at(i), value);
      // else if (((TDigitalScan *)m_scan)->IsUpper())
        // found = GetPreviousParamValue("Bad pixels digital (upper) BB 3V", "Dead Pixels",
                                      // activities.at(i), value);
    // }
    // if (found) prediction->m_nDead += value;
  // }
}


void TDigitalAnalysis::WriteHitData(TScanHisto *histo, int row)
{
  char fName[140];
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TScanResultChip *chipResult = m_result->GetChipResult(m_chipList.at(ichip));
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/Digital_%s_Chip%d.dat", chipResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix(), m_chipList.at(ichip).chipId);
    }
    else {
      sprintf(fName, "Digital_%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(),
              m_chipList.at(ichip).boardIndex, m_chipList.at(ichip).dataReceiver,
              m_chipList.at(ichip).chipId);
    }
    FILE *fp = fopen(fName, "a");
    for (int icol = 0; icol < 1024; icol++) {
      if ((*histo)(m_chipList.at(ichip), icol) > 0) { // write only non-zero values
        fprintf(fp, "%d %d %d\n", icol, row, (int)(*histo)(m_chipList.at(ichip), icol));
      }
    }
    fclose(fp);
  }
}

void TDigitalAnalysis::WriteResult()
{

  // should write to file: Conditions, global, results
  // separate files: stuck pixels (how to separate by HIC?)
  // hitmap file?
  // write both paths to result structure
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    WriteStuckPixels(m_hics.at(ihic));
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/DigitalScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "DigitalScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");
    m_result->WriteToFileGlobal(fp);
    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TDigitalAnalysis::WriteStuckPixels(THic *hic)
{
  char               fName[100];
  TDigitalResultHic *hicResult = (TDigitalResultHic *)m_result->GetHicResult(hic->GetDbId());
  if (m_config->GetUseDataPath()) {
    sprintf(fName, "%s/StuckPixels_%s.dat", hicResult->GetOutputPath().c_str(),
            m_config->GetfNameSuffix());
  }
  else {
    sprintf(fName, "StuckPixels_%s_%s.dat", hic->GetDbId().c_str(), m_config->GetfNameSuffix());
  }

  hicResult->SetStuckFile(fName);
  FILE *               fp     = fopen(fName, "w");
  std::vector<TPixHit> pixels = ((TMaskScan *)m_scan)->GetStuckPixels();

  for (unsigned int i = 0; i < pixels.size(); i++) {
    if (!common::HitBelongsToHic(hic, pixels.at(i))) continue;
    fprintf(fp, "%d %d %d %d\n", pixels.at(i).chipId, pixels.at(i).region, pixels.at(i).dcol,
            pixels.at(i).address);
  }
  fclose(fp);
}

void TDigitalAnalysis::AnalyseHisto(TScanHisto *histo)
{
  int row = histo->GetIndex();
  // std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size()
  //          << std::endl;
  WriteHitData(histo, row);
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (int icol = 0; icol < 1024; icol++) {
      int hits = (int)(*histo)(m_chipList.at(ichip), icol);
      if (hits == m_ninj)
        m_counters.at(ichip).nCorrect++;
      else if (hits > m_ninj)
        m_counters.at(ichip).nNoisy++;
      else if (hits > 0)
        m_counters.at(ichip).nIneff++;
    }
  }
}

void TDigitalAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  TErrorCounter        errCount = ((TMaskScan *)m_scan)->GetErrorCount();
  TDigitalResult *     result   = (TDigitalResult *)m_result;
  std::vector<TPixHit> stuck    = ((TMaskScan *)m_scan)->GetStuckPixels();

  result->m_nTimeout  = errCount.nTimeout;
  result->m_n8b10b    = errCount.n8b10b;
  result->m_nOversize = errCount.nOversizeEvent;
  result->m_nCorrupt  = errCount.nCorruptEvent;

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TDigitalResultChip *chipResult =
        (TDigitalResultChip *)m_result->GetChipResult(m_chipList.at(ichip));

    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;
    chipResult->m_nDead  = 512 * 1024 - (m_counters.at(ichip).nCorrect +
                                        m_counters.at(ichip).nNoisy + m_counters.at(ichip).nIneff);
    chipResult->m_nNoisy = m_counters.at(ichip).nNoisy;
    chipResult->m_nIneff = m_counters.at(ichip).nIneff;
  }

  // for the time being divide stuck pixels on different chips here
  // later: change AlpideDecoder?

  for (unsigned int istuck = 0; istuck < stuck.size(); istuck++) {
    int entry = common::FindIndexForHit(m_chipList, stuck.at(istuck));
    if (entry >= 0) {
      TDigitalResultChip *chipResult =
          (TDigitalResultChip *)m_result->GetChipResult(m_chipList.at(entry));
      chipResult->m_stuck.push_back(stuck.at(istuck));
      chipResult->m_nStuck++;
    }
  }

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TDigitalResultChip *chipResult =
          (TDigitalResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
      TDigitalResultHic *hicResult =
          (TDigitalResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      int nBadChip = chipResult->m_nDead + chipResult->m_nIneff + chipResult->m_nNoisy;
      hicResult->m_nDead += chipResult->m_nDead;
      hicResult->m_nBad += nBadChip;
      if (nBadChip > hicResult->m_nBadWorstChip) hicResult->m_nBadWorstChip = nBadChip;
      hicResult->m_nBadDcols += chipResult->m_nBadDcols;
      hicResult->m_nStuck += chipResult->m_nStuck;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TDigitalResultHic *hicResult =
        (TDigitalResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      hicResult->m_class = GetClassificationIB(hicResult);
    }
    hicResult->SetValidity(true);
    PrintHicClassification(hicResult);
    ComparePrediction(m_hics.at(ihic)->GetDbId());
  }
  WriteResult();

  m_finished = true;
}

// TODO: Add readout errors, requires dividing readout errors by hic (receiver)
// TODO: Make two cuts (red and orange)?
THicClassification TDigitalAnalysis::GetClassificationOB(TDigitalResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  // check data taking variables
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nTimeout, "DIGITAL_MAXTIMEOUT_ORANGE",
        result);
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nCorruptEvent, "DIGITAL_MAXCORRUPT_ORANGE",
        result);

  // chip-wise check
  map<int, TScanResultChip *>::iterator it;
  for (it = result->m_chipResults.begin(); it != result->m_chipResults.end(); it++) {
    TDigitalResultChip *chipResult = (TDigitalResultChip *)it->second;
    int                 chipId     = it->first;
    int                 nBad = chipResult->m_nDead + chipResult->m_nNoisy + chipResult->m_nIneff;
    DoCut(returnValue, CLASS_SILVER, nBad, "DIGITAL_MAXBAD_CHIP_GOLD", result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, nBad, "DIGITAL_MAXBAD_CHIP_SILVER", result, false, chipId);
    DoCut(returnValue, CLASS_RED, nBad, "DIGITAL_MAXBAD_CHIP_BRONZE", result, false, chipId);
  }
  return returnValue;
}

THicClassification TDigitalAnalysis::GetClassificationIB(TDigitalResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  // check data taking variables
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nTimeout, "DIGITAL_MAXTIMEOUT_ORANGE",
        result);
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nCorruptEvent, "DIGITAL_MAXCORRUPT_ORANGE",
        result);

  // chip-wise check
  map<int, TScanResultChip *>::iterator it;
  for (it = result->m_chipResults.begin(); it != result->m_chipResults.end(); it++) {
    TDigitalResultChip *chipResult = (TDigitalResultChip *)it->second;
    int                 chipId     = it->first;
    int                 nBad = chipResult->m_nDead + chipResult->m_nNoisy + chipResult->m_nIneff;
    DoCut(returnValue, CLASS_SILVER, nBad, "DIGITAL_MAXBAD_CHIP_GOLD", result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, nBad, "DIGITAL_MAXBAD_CHIP_SILVER", result, false, chipId);
    DoCut(returnValue, CLASS_RED, nBad, "DIGITAL_MAXBAD_CHIP_BRONZE", result, false, chipId);
  }
  return returnValue;
}

void TDigitalResult::WriteToFileGlobal(FILE *fp)
{
  fprintf(fp, "8b10b errors:\t%d\n", m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n", m_nCorrupt);
  fprintf(fp, "Oversized events:\t%d\n", m_nOversize);
  fprintf(fp, "Timeouts:\t%d\n", m_nTimeout);
}

void TDigitalResultHic::GetParameterSuffix(std::string &suffix, std::string &file_suffix)
{
  if (m_backBias == 0) {
    if (m_nominal) {
      suffix      = string(" (nominal)");
      file_suffix = string("_nominal");
    }
    else if (m_lower) {
      suffix      = string(" (lower)");
      file_suffix = string("_lower");
    }
    else if (m_upper) {
      suffix      = string(" (upper)");
      file_suffix = string("_upper");
    }
  }
  else if (fabs(m_backBias - 3) < 0.01) {
    if (m_nominal) {
      suffix      = string(" (nominal) BB 3V");
      file_suffix = string("_nominal_BB3V");
    }
    else if (m_lower) {
      suffix      = string(" (lower) BB 3V");
      file_suffix = string("_lower_BB3V");
    }
    else if (m_upper) {
      suffix      = string(" (upper) BB 3V");
      file_suffix = string("_upper_BB3V");
    }
  }
}

// void TDigitalResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string suffix, file_suffix, fileName, remoteName;
  // GetParameterSuffix(suffix, file_suffix);
  // DbAddParameter(db, activity, string("Timeouts digital") + suffix, (float)m_errorCounter.nTimeout,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("8b10b errors digital") + suffix,
                //  (float)m_errorCounter.n8b10b, GetParameterFile());
  // DbAddParameter(db, activity, string("Corrupt events digital") + suffix,
                //  (float)m_errorCounter.nCorruptEvent, GetParameterFile());
  // DbAddParameter(db, activity, string("Priority encoder errors digital") + suffix,
                //  (float)m_errorCounter.nPrioEncoder, GetParameterFile());
  // DbAddParameter(db, activity, string("Bad double columns digital") + suffix, (float)m_nBadDcols,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("Bad pixels digital") + suffix, (float)m_nBad,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("Bad pixels digital, worst chip") + suffix,
                //  (float)m_nBadWorstChip, GetParameterFile());
  // DbAddParameter(db, activity, string("Increase in dead pixels digital") + suffix,
                //  (float)m_nDeadIncrease, GetParameterFile());

  // std::size_t slash = string(m_resultFile).find_last_of("/");
  // fileName          = string(m_resultFile).substr(slash + 1); // strip path
  // std::size_t point = fileName.find_last_of(".");
  // remoteName        = fileName.substr(0, point) + file_suffix + ".dat";
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
// }

void TDigitalResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Bad pixels:      %d\n", m_nBad);
  fprintf(fp, "Bad double cols: %d\n", m_nBadDcols);
  fprintf(fp, "Stuck pixels:    %d\n", m_nStuck);

  fprintf(fp, "Dead pixels:     %d\n", m_nDead);
  fprintf(fp, "   Increase:     %d\n", m_nDeadIncrease);

  fprintf(fp, "\nStuck pixel file: %s\n", m_stuckFile);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }

  std::cout << std::endl << "Error counts (Test feature): " << std::endl;
  std::cout << "8b10b errors:  " << m_errorCounter.n8b10b << std::endl;
  std::cout << "corrupt events " << m_errorCounter.nCorruptEvent << std::endl;
  std::cout << "timeouts:      " << m_errorCounter.nTimeout << std::endl;
}

void TDigitalResultHic::Compare(TScanResultHic *aPrediction)
{
  TDigitalResultHic *prediction = (TDigitalResultHic *)aPrediction;
  m_nDeadIncrease               = m_nDead - prediction->m_nDead;
  std::cout << "expected " << prediction->m_nDead << " dead pixels, found " << m_nDead << std::endl;
}


void TDigitalResultChip::WriteToFile(FILE *fp)
{
  fprintf(fp, "Dead pixels:        %d\n", m_nDead);
  fprintf(fp, "Inefficient pixels: %d\n", m_nIneff);
  fprintf(fp, "Noisy pixels:       %d\n", m_nNoisy);
  fprintf(fp, "Bad double cols:    %d\n", m_nBadDcols);
  fprintf(fp, "Stuck pixels:       %d\n", m_nStuck);
}

float TDigitalResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  case deadPix:
    return (float)m_nDead;
  case noisyPix:
    return (float)m_nNoisy;
  case ineffPix:
    return (float)m_nIneff;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

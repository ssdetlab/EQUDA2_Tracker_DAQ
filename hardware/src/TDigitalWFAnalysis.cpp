#include <iostream>

// #include "DBHelpers.h"
#include "TDigitalWFAnalysis.h"

TDigitalWFAnalysis::TDigitalWFAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                       TScanConfig *aScanConfig, std::vector<THic *> hics,
                                       std::mutex *aMutex, TDigitalWFResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  m_ninj = m_config->GetParamValue("NINJ");
  if (aResult)
    m_result = aResult;
  else
    m_result = new TDigitalWFResult();
  FillVariableList();
}

void TDigitalWFAnalysis::FillVariableList()
{
  m_variableList.insert(
      std::pair<const char *, TResultVariable>("# of unmaskable Pixels", unmaskablePix));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# of stuck Pixels", stuckPix));
}

void TDigitalWFAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}

void TDigitalWFAnalysis::InitCounters()
{
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TDigitalWFResultChip *result =
        (TDigitalWFResultChip *)m_result->GetChipResult(m_chipList.at(i));
    result->m_nStuck      = 0;
    result->m_nBadDCol    = 0;
    result->m_nUnmaskable = 0;
  }

  std::map<std::string, TScanResultHic *>::iterator it;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TDigitalWFResultHic *result = (TDigitalWFResultHic *)it->second;
    result->m_backBias          = m_scan->GetBackbias();
    result->m_nStuck            = 0;
    result->m_nBadDCol          = 0;
    result->m_nUnmaskable       = 0;
  }
}

void TDigitalWFAnalysis::AnalyseHisto(TScanHisto *histo)
{
  int row = histo->GetIndex();
  // std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size()
  //          << std::endl;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TDigitalWFResultChip *result =
        (TDigitalWFResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    for (int icol = 0; icol < 1024; icol++) {
      int hits = (int)(*histo)(m_chipList.at(ichip), icol);
      if (hits > 0) {
        result->m_nUnmaskable++;
        int     dcol = icol / 2;
        TPixHit hit;
        hit.boardIndex = m_chipList.at(ichip).boardIndex;
        hit.channel    = m_chipList.at(ichip).dataReceiver;
        hit.chipId     = m_chipList.at(ichip).chipId;
        hit.dcol       = dcol % 16;
        hit.region     = dcol / 16;
        hit.address    = row * 2;
        if (row % 2) {
          hit.address += (1 - (icol % 2));
        }
        else {
          hit.address += (icol % 2);
        }
        m_unmaskable.push_back(hit);
      }
    }
  }
}

void TDigitalWFAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  TErrorCounter        errCount = ((TMaskScan *)m_scan)->GetErrorCount();
  TDigitalWFResult *   result   = (TDigitalWFResult *)m_result;
  std::vector<TPixHit> stuck    = ((TMaskScan *)m_scan)->GetStuckPixels();

  result->m_nTimeout  = errCount.nTimeout;
  result->m_n8b10b    = errCount.n8b10b;
  result->m_nOversize = errCount.nOversizeEvent;
  result->m_nCorrupt  = errCount.nCorruptEvent;

  // for the time being divide stuck pixels on different chips here
  // later: change AlpideDecoder?

  for (unsigned int istuck = 0; istuck < stuck.size(); istuck++) {
    int entry = common::FindIndexForHit(m_chipList, stuck.at(istuck));
    if (entry >= 0) {
      TDigitalWFResultChip *chipResult =
          (TDigitalWFResultChip *)m_result->GetChipResult(m_chipList.at(entry));
      chipResult->m_stuck.push_back(stuck.at(istuck));
      chipResult->m_nStuck++;
      if (!common::DColAlreadyHit(&(chipResult->m_stuckOnePerDCol), stuck.at(istuck))) {
        chipResult->m_stuckOnePerDCol.push_back(stuck.at(istuck));
        chipResult->m_nBadDCol++;
      }
    }
  }

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TDigitalWFResultChip *chipResult =
          (TDigitalWFResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
      TDigitalWFResultHic *hicResult =
          (TDigitalWFResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      hicResult->m_nStuck += chipResult->m_nStuck;
      hicResult->m_nUnmaskable += chipResult->m_nUnmaskable;
      hicResult->m_nBadDCol += chipResult->m_nBadDCol;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TDigitalWFResultHic *hicResult =
        (TDigitalWFResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    hicResult->SetValidity(true);
    PrintHicClassification(hicResult);
  }
  WriteResult();

  m_finished = true;
}

// TODO: Add readout errors, requires dividing readout errors by hic (receiver)
// TODO: Make two cuts (red and orange)?
THicClassification TDigitalWFAnalysis::GetClassificationOB(TDigitalWFResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  // chip-wise check
  map<int, TScanResultChip *>::iterator it;
  for (it = result->m_chipResults.begin(); it != result->m_chipResults.end(); it++) {
    TDigitalWFResultChip *chipResult = (TDigitalWFResultChip *)it->second;
    int                   chipId     = it->first;
    DoCut(returnValue, CLASS_SILVER, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_GOLD",
          result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_SILVER",
          result, false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_BRONZE",
          result, false, chipId);
  }
  return returnValue;
}

THicClassification TDigitalWFAnalysis::GetClassificationIB(TDigitalWFResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  // chip-wise check
  map<int, TScanResultChip *>::iterator it;
  for (it = result->m_chipResults.begin(); it != result->m_chipResults.end(); it++) {
    TDigitalWFResultChip *chipResult = (TDigitalWFResultChip *)it->second;
    int                   chipId     = it->first;

    DoCut(returnValue, CLASS_SILVER, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_GOLD",
          result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_SILVER",
          result, false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nBadDCol, "DIGITAL_MAXNOMASKSTUCK_CHIP_BRONZE",
          result, false, chipId);
  }
  return returnValue;
}

void TDigitalWFAnalysis::WriteResult()
{

  // should write to file: Conditions, global, results
  // separate files: stuck pixels (how to separate by HIC?)
  // write both paths to result structure
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    WriteStuckPixels(m_hics.at(ihic));
    WriteUnmaskedPixels(m_hics.at(ihic));

    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/DigitalWFScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "DigitalWFScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");
    m_result->WriteToFileGlobal(fp);
    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);
    //    m_result->WriteToFile     (fName);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TDigitalWFAnalysis::WriteStuckPixels(THic *hic)
{
  char            fName[100];
  TScanResultHic *hicResult = m_result->GetHicResult(hic->GetDbId());
  if (m_config->GetUseDataPath()) {
    sprintf(fName, "%s/StuckPixels_%s.dat", hicResult->GetOutputPath().c_str(),
            m_config->GetfNameSuffix());
  }
  else {
    sprintf(fName, "StuckPixels_%s_%s.dat", hic->GetDbId().c_str(), m_config->GetfNameSuffix());
  }

  ((TDigitalWFResultHic *)m_result->GetHicResult(hic->GetDbId()))->SetStuckFile(fName);
  std::vector<TPixHit> pixels = ((TMaskScan *)m_scan)->GetStuckPixels();

  WritePixels(hic, pixels, fName);
}

void TDigitalWFAnalysis::WriteUnmaskedPixels(THic *hic)
{
  char            fName[100];
  TScanResultHic *hicResult = m_result->GetHicResult(hic->GetDbId());
  if (m_config->GetUseDataPath()) {
    sprintf(fName, "%s/UnmaskedPixels_%s.dat", hicResult->GetOutputPath().c_str(),
            m_config->GetfNameSuffix());
  }
  else {
    sprintf(fName, "UnmaskedPixels_%s_%s.dat", hic->GetDbId().c_str(), m_config->GetfNameSuffix());
  }

  ((TDigitalWFResultHic *)m_result->GetHicResult(hic->GetDbId()))->SetUnmaskedFile(fName);
  std::vector<TPixHit> pixels = m_unmaskable;

  WritePixels(hic, pixels, fName);
}

void TDigitalWFAnalysis::WritePixels(THic *hic, std::vector<TPixHit> pixels, const char *fName)
{
  FILE *fp = fopen(fName, "w");

  for (unsigned int i = 0; i < pixels.size(); i++) {
    if (!common::HitBelongsToHic(hic, pixels.at(i))) continue;
    fprintf(fp, "%d %d %d %d\n", pixels.at(i).chipId, pixels.at(i).region, pixels.at(i).dcol,
            pixels.at(i).address);
  }
  fclose(fp);
}

void TDigitalWFResult::WriteToFileGlobal(FILE *fp)
{
  fprintf(fp, "8b10b errors:\t%d\n", m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n", m_nCorrupt);
  fprintf(fp, "Oversized events:\t%d\n", m_nOversize);
  fprintf(fp, "Timeouts:\t%d\n", m_nTimeout);
}

// void TDigitalWFResult::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::map<std::string, TScanResultHic *>::iterator it;
  // for (it = m_hicResults.begin(); it != m_hicResults.end(); it++) {
    // TDigitalWFResultHic *hicResult = (TDigitalWFResultHic *)it->second;
    // hicResult->WriteToDB(db, activity);
  // }
// }


void TDigitalWFResultHic::GetParameterSuffix(std::string &suffix, std::string &file_suffix)
{
  if (m_backBias == 0) {
    suffix      = string("");
    file_suffix = string("");
  }
  else if (fabs(m_backBias - 3) < 0.01) {
    suffix      = string(" BB 3V");
    file_suffix = string("_BB_3V");
  }
}


// void TDigitalWFResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // string remoteName, fileName, suffix, file_suffix;
  // GetParameterSuffix(suffix, file_suffix);
  // DbAddParameter(db, activity, string("Unmaskable pixels") + suffix, (float)m_nUnmaskable,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("Unmaskable stuck pixels") + suffix, (float)m_nBadDCol,
                //  GetParameterFile());

  // std::size_t slash = string(m_resultFile).find_last_of("/");
  // fileName          = string(m_resultFile).substr(slash + 1); // strip path
  // std::size_t point = fileName.find_last_of(".");
  // remoteName        = fileName.substr(0, point) + file_suffix + ".dat";
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
// }

void TDigitalWFResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Unmaskable pixels:             %d\n", m_nUnmaskable);
  fprintf(fp, "of which stuck:                %d\n", m_nStuck);
  fprintf(fp, "- counting only one per dcol:  %d\n", m_nBadDCol);

  fprintf(fp, "\nStuck pixel file: %s\n", m_stuckFile);
  fprintf(fp, "\nUnmaskable pixel file: %s\n", m_unmaskedFile);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

void TDigitalWFResultChip::WriteToFile(FILE *fp)
{
  fprintf(fp, "Unmaskable pixels:             %d\n", m_nUnmaskable);
  fprintf(fp, "stuck pixels:                  %d\n", m_nStuck);
  fprintf(fp, "- counting only one per dcol:  %d\n", m_nBadDCol);
}

float TDigitalWFResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  case unmaskablePix:
    return (float)m_nUnmaskable;
  case stuckPix:
    return (float)m_nStuck;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

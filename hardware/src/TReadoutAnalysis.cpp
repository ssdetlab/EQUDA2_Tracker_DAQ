#include "TReadoutAnalysis.h"
// #include "DBHelpers.h"
#include "TReadoutTest.h"
#include <iostream>
#include <string>
#include <vector>

TReadoutAnalysis::TReadoutAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                   TScanConfig *aScanConfig, std::vector<THic *> hics,
                                   std::mutex *aMutex, TReadoutResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  m_nTrig = m_config->GetParamValue("NTRIG");
  m_occ   = m_config->GetParamValue("READOUTOCC");
  m_row   = ((TReadoutTest *)m_scan)->GetRow();
  if (aResult)
    m_result = aResult;
  else
    m_result = new TReadoutResult();
  FillVariableList();
}

void TReadoutAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}

void TReadoutAnalysis::InitCounters()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TReadoutResultHic *hicResult =
        (TReadoutResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    hicResult->m_linkSpeed   = ((TReadoutTest *)m_scan)->GetLinkSpeed();
    hicResult->m_driver      = ((TReadoutTest *)m_scan)->GetDriver();
    hicResult->m_preemp      = ((TReadoutTest *)m_scan)->GetPreemp();
    hicResult->m_missingHits = 0;
    hicResult->m_deadPixels  = 0;
    hicResult->m_ineffPixels = 0;
    hicResult->m_extraHits   = 0;
    hicResult->m_noisyPixels = 0;
  }

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TReadoutResultChip *chipResult =
        (TReadoutResultChip *)m_result->GetChipResult(m_chipList.at(i));
    chipResult->m_missingHits = 0;
    chipResult->m_deadPixels  = 0;
    chipResult->m_ineffPixels = 0;
    chipResult->m_extraHits   = 0;
    chipResult->m_noisyPixels = 0;
  }
}

bool TReadoutAnalysis::IsInjected(int col, int row)
{
  if (row != m_row) return false; // wrong row
  if (m_occ == 32) return true;   // correct row, all pixels in row injected

  int colStep = 32 / m_occ;
  if (col % colStep == 0) return true;
  return false;
}

void TReadoutAnalysis::AnalyseHisto(TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TReadoutResultChip *chipResult =
        (TReadoutResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    // int                 channel    = m_chipList.at(ichip).dataReceiver;
    // int                 chipId     = m_chipList.at(ichip).chipId;
    // int                 boardIndex = m_chipList.at(ichip).boardIndex;

    if (!chipResult) {
      std::cout << "Warning (TReadoutAnalysis): Missing chip result" << std::endl;
      continue;
    }

    for (int icol = 0; icol < 1024; icol++) {
      for (int irow = 0; irow < 512; irow++) {
        // check if pixel selected; if yes -> has to have NTRIG hits, if not has to have 0
        if (IsInjected(icol, irow)) {
          if ((*histo)(m_chipList.at(ichip), icol, irow) == 0) {
            chipResult->m_missingHits += m_nTrig;
            chipResult->m_deadPixels++;
          }
          else if (!((*histo)(m_chipList.at(ichip), icol, irow) == m_nTrig)) {
            chipResult->m_missingHits += m_nTrig - (*histo)(m_chipList.at(ichip), icol, irow);
            chipResult->m_ineffPixels++;
          }
        }
        else {
          if ((*histo)(m_chipList.at(ichip), icol, irow) != 0) {
            chipResult->m_extraHits += (*histo)(m_chipList.at(ichip), icol, irow);
            chipResult->m_noisyPixels++;
          }
        }
      }
    }
  }
}

void TReadoutAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TReadoutResultHic *hicResult =
        (TReadoutResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
    std::cout << "8b10b errors: " << hicResult->m_errorCounter.n8b10b << std::endl;
  }

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TReadoutResultChip *chipResult =
          (TReadoutResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
      TReadoutResultHic *hicResult =
          (TReadoutResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());

      hicResult->m_missingHits += chipResult->m_missingHits;
      hicResult->m_deadPixels += chipResult->m_deadPixels;
      hicResult->m_ineffPixels += chipResult->m_ineffPixels;
      hicResult->m_extraHits += chipResult->m_extraHits;
      hicResult->m_noisyPixels += chipResult->m_noisyPixels;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TReadoutResultHic *hicResult =
        (TReadoutResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      hicResult->m_class = GetClassificationIB(hicResult);
    }
    PrintHicClassification(hicResult);
    hicResult->SetValidity(true);
  }
  WriteResult();

  m_finished = true;
}

void TReadoutAnalysis::WriteResult()
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/ReadoutScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "ReadoutScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));
    ((TReadoutTest *)m_scan)->WritePLLReg(fName, m_hics.at(ihic));

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

// TODO: make configurable cuts? different for OB/IB?
// Different depending on driver setting? (e.g. if errors at 10 -> Red, if errors at 2 -> Orange)?

THicClassification TReadoutAnalysis::GetClassificationOB(TReadoutResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nCorruptEvent, "READOUT_MAXCORRUPT", result);
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nTimeout, "READOUT_MAXTIMEOUT", result);
  DoCut(returnValue, CLASS_SILVER, result->m_errorCounter.n8b10b, "READOUT_MAX8b10b_GREEN", result);

  return returnValue;
}

THicClassification TReadoutAnalysis::GetClassificationIB(TReadoutResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nCorruptEvent, "READOUT_MAXCORRUPT", result);
  DoCut(returnValue, CLASS_RED, result->m_errorCounter.nTimeout, "READOUT_MAXTIMEOUT", result);
  DoCut(returnValue, CLASS_SILVER, result->m_errorCounter.n8b10b, "READOUT_MAX8b10b_GREEN", result);

  return returnValue;
}

void TReadoutResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "== Scan parameters:\n");
  fprintf(fp, "Voltage Scale:   %.1f\n", ((TReadoutParameters *)m_scanParameters)->voltageScale);
  fprintf(fp, "PLL Stages:      %d\n", ((TReadoutParameters *)m_scanParameters)->pllStages);
  fprintf(fp, "LinkSpeed:       %d\n", ((TReadoutParameters *)m_scanParameters)->linkSpeed);
  fprintf(fp, "Driver Strength: %d\n", ((TReadoutParameters *)m_scanParameters)->driverStrength);
  fprintf(fp, "Preemphasis:     %d\n", ((TReadoutParameters *)m_scanParameters)->preemp);
  fprintf(fp, "==\n\n");

  fprintf(fp, "8b10b errors:   %d\n", m_errorCounter.n8b10b);
  fprintf(fp, "Corrupt events: %d\n", m_errorCounter.nCorruptEvent);
  fprintf(fp, "Timeouts:       %d\n", m_errorCounter.nTimeout);

  fprintf(fp, "Missing hits:       %d\n", m_missingHits);
  fprintf(fp, "Extra hits:         %d\n", m_extraHits);
  fprintf(fp, "Dead pixels:        %d\n", m_deadPixels);
  fprintf(fp, "Inefficient pixels: %d\n", m_ineffPixels);
  fprintf(fp, "Noisy pixels:       %d\n", m_noisyPixels);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

void TReadoutResultHic::GetParameterSuffix(std::string &suffix, std::string &file_suffix)
{
  TReadoutParameters *params = (TReadoutParameters *)m_scanParameters;
  char                scale[5];

  sprintf(scale, "%.1f", params->voltageScale);

  if ((params->voltageScale != 1) || (params->pllStages >= 0)) {
    suffix = string(" ") + to_string(m_linkSpeed) + string(" ") + string(scale) + string("xDVDD") +
             string(" ") + to_string(params->pllStages);
  }
  else {
    suffix = string(" ") + to_string(m_linkSpeed);
  }
  suffix += string(" D") + to_string(m_driver);
  suffix += string(" P") + to_string(m_preemp);

  if ((params->voltageScale != 1) || (params->pllStages >= 0)) {
    file_suffix = string("_") + to_string(m_linkSpeed) + string("_") + string(scale) +
                  string("xDVDD") + string("_") + to_string(params->pllStages);
  }
  else {
    file_suffix = string("_") + to_string(m_linkSpeed);
  }
  file_suffix += string("_D") + to_string(m_driver);
  file_suffix += string("_P") + to_string(m_preemp);
}

// void TReadoutResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string suffix, file_suffix, fileName, remoteName;
  // GetParameterSuffix(suffix, file_suffix);
  // DbAddParameter(db, activity, string("Timeouts readout") + suffix, (float)m_errorCounter.nTimeout,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("8b10b errors readout") + suffix,
                //  (float)m_errorCounter.n8b10b, GetParameterFile());
  // DbAddParameter(db, activity, string("Corrupt events readout") + suffix,
                //  (float)m_errorCounter.nCorruptEvent, GetParameterFile());
  // DbAddParameter(db, activity, string("Bad pixels readout") + suffix,
                //  (float)(m_deadPixels + m_ineffPixels + m_noisyPixels), GetParameterFile());

  // std::size_t slash = string(m_resultFile).find_last_of("/");
  // fileName          = string(m_resultFile).substr(slash + 1); // strip path
  // std::size_t point = fileName.find_last_of(".");
  // remoteName        = fileName.substr(0, point) + file_suffix + ".dat";
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
// }

float TReadoutResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TReadoutResultChip::WriteToFile(FILE *fp)
{
  fprintf(fp, "Missing hits:       %d\n", m_missingHits);
  fprintf(fp, "Extra hits:         %d\n", m_extraHits);
  fprintf(fp, "Dead pixels:        %d\n", m_deadPixels);
  fprintf(fp, "Inefficient pixels: %d\n", m_ineffPixels);
  fprintf(fp, "Noisy pixels:       %d\n", m_noisyPixels);
}

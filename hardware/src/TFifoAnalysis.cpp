#include "TFifoAnalysis.h"
#include "AlpideConfig.h"
// #include "DBHelpers.h"
#include "TFifoTest.h"
#include <iostream>
#include <string>
#include <vector>

// TODO: Add number of exceptions to result
// TODO: Add errors per region to chip result

TFifoAnalysis::TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                             TScanConfig *aScanConfig, std::vector<THic *> hics, std::mutex *aMutex,
                             TFifoResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TFifoResult();

  FillVariableList();
}

void TFifoAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
}

void TFifoAnalysis::FillVariableList()
{
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0x0000", Err0));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0xffff", Errf));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0x5555", Err5));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0xaaaa", Erra));
}

string TFifoAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBQualification:
    return string("ALPIDEB Chip Testing Analysis");
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Endurance Test");
  case OBHalfStaveOL:
    return string("OB HIC Reception Test");
  case OBHalfStaveML:
    return string("OB HIC Reception Test");
  case IBQualification:
    return string("ALPIDEB Chip Testing Analysis");
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

void TFifoAnalysis::InitCounters()
{
  m_counters.clear();

  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TFifoCounter counter;
    counter.boardIndex = m_chipList.at(i).boardIndex;
    counter.receiver   = m_chipList.at(i).dataReceiver;
    counter.chipId     = m_chipList.at(i).chipId;
    counter.err0       = 0;
    counter.err5       = 0;
    counter.erra       = 0;
    counter.errf       = 0;
    counter.exc        = 0;
    m_counters.push_back(counter);
  }

  std::map<std::string, TScanResultHic *>::iterator it;
  std::map<int, TScanResultChip *>::iterator        itChip;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TFifoResultHic *result = (TFifoResultHic *)it->second;
    result->m_exc          = 0;
    result->m_nFaultyChips = 0;
    result->m_err0         = 0;
    result->m_err5         = 0;
    result->m_erra         = 0;
    result->m_errf         = 0;
    result->m_lower        = ((TFifoTest *)m_scan)->IsLower();
    result->m_upper        = ((TFifoTest *)m_scan)->IsUpper();
    result->m_nominal      = ((TFifoTest *)m_scan)->IsNominal();
    result->m_driver       = ((TFifoTest *)m_scan)->GetDriver();
    for (itChip = result->m_chipResults.begin(); itChip != result->m_chipResults.end(); ++itChip) {
      TFifoResultChip *resultChip = (TFifoResultChip *)itChip->second;
      resultChip->m_err0          = 0;
      resultChip->m_err5          = 0;
      resultChip->m_erra          = 0;
      resultChip->m_errf          = 0;
      resultChip->m_exc           = 0;
    }
  }
}

void TFifoAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/FifoScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "FifoScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");

    hicResult->SetResultFile(fName);
    hicResult->WriteToFile(fp);
    fclose(fp);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TFifoAnalysis::AnalyseHisto(TScanHisto *histo)
{
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    for (int ireg = 0; ireg < 32; ireg++) {
      m_counters.at(ichip).err0 += (int)((*histo)(m_chipList.at(ichip), ireg, 0x0));
      m_counters.at(ichip).err5 += (int)((*histo)(m_chipList.at(ichip), ireg, 0x5));
      m_counters.at(ichip).erra += (int)((*histo)(m_chipList.at(ichip), ireg, 0xa));
      m_counters.at(ichip).errf += (int)((*histo)(m_chipList.at(ichip), ireg, 0xf));
    }
    m_counters.at(ichip).exc += (int)((*histo)(m_chipList.at(ichip), 32, 0));
  }
}

void TFifoAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TFifoResultChip *chipResult = (TFifoResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;

    chipResult->m_err0 = m_counters.at(ichip).err0;
    chipResult->m_err5 = m_counters.at(ichip).err5;
    chipResult->m_erra = m_counters.at(ichip).erra;
    chipResult->m_errf = m_counters.at(ichip).errf;
    chipResult->m_exc  = m_counters.at(ichip).exc;

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TFifoResultHic *hicResult =
          (TFifoResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      hicResult->m_err0 += chipResult->m_err0;
      hicResult->m_err5 += chipResult->m_err5;
      hicResult->m_erra += chipResult->m_erra;
      hicResult->m_errf += chipResult->m_errf;
      hicResult->m_exc += chipResult->m_exc;

      if (chipResult->m_err0 + chipResult->m_err5 + chipResult->m_erra + chipResult->m_errf > 0)
        hicResult->m_nFaultyChips++;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TFifoResultHic *hicResult =
        (TFifoResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    hicResult->m_class = GetClassification(hicResult);
    PrintHicClassification(hicResult);
    hicResult->SetValidity(true);
  }

  WriteResult();

  m_finished = true;
}

THicClassification TFifoAnalysis::GetClassification(TFifoResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_RED, result->m_exc, "FIFO_MAXEXCEPTION", result);

  // DoCut(returnValue, CLASS_SILVER, result->m_err0, "FIFO_MAXERR_GREEN", result);
  // DoCut(returnValue, CLASS_SILVER, result->m_err5, "FIFO_MAXERR_GREEN", result);
  // DoCut(returnValue, CLASS_SILVER, result->m_erra, "FIFO_MAXERR_GREEN", result);
  // DoCut(returnValue, CLASS_SILVER, result->m_errf, "FIFO_MAXERR_GREEN", result);
  DoCut(returnValue, CLASS_RED, result->m_err0, "FIFO_MAXERR_ORANGE", result);
  DoCut(returnValue, CLASS_RED, result->m_err5, "FIFO_MAXERR_ORANGE", result);
  DoCut(returnValue, CLASS_RED, result->m_erra, "FIFO_MAXERR_ORANGE", result);
  DoCut(returnValue, CLASS_RED, result->m_errf, "FIFO_MAXERR_ORANGE", result);

  // DoCut(returnValue, CLASS_RED, result->m_errf, "FIFO_MAXFAULTYCHIPS", result);
  return returnValue;
}

void TFifoResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Exceptions: %d\n\n", m_exc);

  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

void TFifoResultHic::GetParameterSuffix(std::string &suffix, std::string &file_suffix)
{
  if (m_nominal) {
    if (m_driver != ChipConfig::DCTRL_DRIVER) {
      suffix      = string(" (Driver ") + to_string(m_driver) + string(")");
      file_suffix = string("_Driver_") + to_string(m_driver);
    }
    else {
      suffix      = string(" (nominal)");
      file_suffix = string("_nominal");
    }
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

;// void TFifoResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string suffix, file_suffix, fileName, remoteName;
  // GetParameterSuffix(suffix, file_suffix);

  // DbAddParameter(db, activity, string("FIFO errors") + suffix,
                //  (float)(m_err0 + m_err5 + m_erra + m_errf), GetParameterFile());
  // DbAddParameter(db, activity, string("FIFO exceptions") + suffix, (float)m_exc,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("Chips with FIFO errors") + suffix, (float)m_nFaultyChips,
                //  GetParameterFile());

  // std::size_t slash = string(m_resultFile).find_last_of("/");
  // fileName          = string(m_resultFile).substr(slash + 1); // strip path
  // std::size_t point = fileName.find_last_of(".");
  // remoteName        = fileName.substr(0, point) + file_suffix + ".dat";
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
// }

float TFifoResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  case Err0:
    return (float)m_err0;
  case Err5:
    return (float)m_err5;
  case Erra:
    return (float)m_erra;
  case Errf:
    return (float)m_errf;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TFifoResultChip::WriteToFile(FILE *fp)
{
  fprintf(fp, "Exceptions: %d\n", m_exc);
  fprintf(fp, "Errors in pattern 0x0000: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5555: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xaaaa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xffff: %d\n", m_errf);
}

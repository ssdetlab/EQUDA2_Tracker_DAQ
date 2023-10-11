#include "TLocalBusAnalysis.h"

TLocalBusAnalysis::TLocalBusAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                     TScanConfig *aScanConfig, std::vector<THic *> hics,
                                     std::mutex *aMutex, TLocalBusResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TLocalBusResult();

  FillVariableList();
}

void TLocalBusAnalysis::InitCounters()
{
  std::map<std::string, TScanResultHic *>::iterator it;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TLocalBusResultHic *result = (TLocalBusResultHic *)it->second;
    result->m_err0             = 0;
    result->m_err5             = 0;
    result->m_erra             = 0;
    result->m_errf             = 0;
    result->m_errBusyOn        = 0;
    result->m_errBusyOff       = 0;
    std::map<int, TScanResultChip *>::iterator itChip;
    for (itChip = result->m_chipResults.begin(); itChip != result->m_chipResults.end(); ++itChip) {
      TLocalBusResultChip *resultChip = (TLocalBusResultChip *)itChip->second;
      resultChip->m_err0              = 0;
      resultChip->m_err5              = 0;
      resultChip->m_erra              = 0;
      resultChip->m_errf              = 0;
      resultChip->m_errBusyOn         = 0;
      resultChip->m_errBusyOff        = 0;
    }
  }
}

void TLocalBusAnalysis::Initialize()
{
  std::cout << "m_scan = " << m_scan << std::endl;
  ReadChipList();
  CreateHicResults();
}

void TLocalBusAnalysis::FillVariableList()
{
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0x0000", Err0));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0xffff", Errf));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0x5555", Err5));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Errors 0xaaaa", Erra));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Busy On Errors", ErrBusyOn));
  m_variableList.insert(std::pair<const char *, TResultVariable>("# Busy Off Errors", ErrBusyOff));
}

void TLocalBusAnalysis::AnalyseHisto(TScanHisto *histo)
{
  common::TChipIndex   idx;
  TLocalBusResultChip *result;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    idx    = m_chipList.at(ichip);
    result = (TLocalBusResultChip *)m_result->GetChipResult(idx);
    for (int second = 0; second < 15; second++) {
      result->m_err0 += (*histo)(idx, second, 0x0);
      result->m_err5 += (*histo)(idx, second, 0x5);
      result->m_erra += (*histo)(idx, second, 0xa);
      result->m_errf += (*histo)(idx, second, 0xf);
      result->m_errBusyOff += (*histo)(idx, second, 16);
      result->m_errBusyOff += (*histo)(idx, second, 17);
    }
  }
}

void TLocalBusAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->IsValid()) continue;
    sprintf(fName, "LocalBusScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
            m_config->GetfNameSuffix());
    m_scan->WriteConditions(fName, m_hics.at(ihic));

    FILE *fp = fopen(fName, "a");

    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->SetResultFile(fName);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->WriteToFile(fp);
    fclose(fp);

    m_scan->WriteChipRegisters(fName);
    m_scan->WriteBoardRegisters(fName);
    m_scan->WriteTimestampLog(fName);
  }
}

void TLocalBusAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TLocalBusResultChip *chipResult =
        (TLocalBusResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) std::cout << "WARNING: chipResult = 0" << std::endl;
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TLocalBusResultHic *hicResult =
          (TLocalBusResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      hicResult->m_err0 += chipResult->m_err0;
      hicResult->m_err5 += chipResult->m_err5;
      hicResult->m_erra += chipResult->m_erra;
      hicResult->m_errf += chipResult->m_errf;
      hicResult->m_errBusyOff += chipResult->m_errBusyOff;
      hicResult->m_errBusyOn += chipResult->m_errBusyOn;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    int                 Total = 0;
    TLocalBusResultHic *hicResult =
        (TLocalBusResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());

    Total += hicResult->m_err0;
    Total += hicResult->m_err5;
    Total += hicResult->m_erra;
    Total += hicResult->m_errf;
    Total += hicResult->m_errBusyOn;
    Total += hicResult->m_errBusyOff;

    if (Total >= m_config->GetLocalBusCutRed())
      hicResult->m_class = CLASS_RED;
    else
      hicResult->m_class = CLASS_GOLD;
    hicResult->SetValidity(true);
  }

  WriteResult();
  m_finished = true;
}

void TLocalBusResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Errors in pattern 0x0: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xf: %d\n\n", m_errf);

  fprintf(fp, "Errors for Busy on:  %d\n", m_errBusyOn);
  fprintf(fp, "Errors for Busy off: %d\n", m_errBusyOff);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

float TLocalBusResultChip::GetVariable(TResultVariable var)
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
  case ErrBusyOn:
    return (float)m_errBusyOn;
  case ErrBusyOff:
    return (float)m_errBusyOff;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TLocalBusResultChip::WriteToFile(FILE *fp)
{
  fprintf(fp, "Errors in pattern 0x0: %d\n", m_err0);
  fprintf(fp, "Errors in pattern 0x5: %d\n", m_err5);
  fprintf(fp, "Errors in pattern 0xa: %d\n", m_erra);
  fprintf(fp, "Errors in pattern 0xf: %d\n\n", m_errf);

  fprintf(fp, "Errors for Busy on:  %d\n", m_errBusyOn);
  fprintf(fp, "Errors for Busy off: %d\n", m_errBusyOff);
}

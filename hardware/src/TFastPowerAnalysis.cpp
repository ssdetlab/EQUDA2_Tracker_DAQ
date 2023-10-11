#include "TFastPowerAnalysis.h"
// #include "DBHelpers.h"
#include "TPowerTest.h"
#ifdef HAS_ROOT
#include "TCanvas.h"
#include "TError.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#endif

#include <string>

TFastPowerAnalysis::TFastPowerAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                       TScanConfig *aScanConfig, std::vector<THic *> hics,
                                       std::mutex *aMutex, TFastPowerResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TFastPowerResult();
  m_ivPoints = m_config->GetParamValue("IVPOINTS");
}

string TFastPowerAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBPower:
    return string("OB HIC Reception Test");
  default:
    return string("");
  }
}

void TFastPowerAnalysis::CreateIVHisto(TFastPowerResultHic *hicResult)
{
#ifdef HAS_ROOT
  gErrorIgnoreLevel = kWarning; // remove TCanvas::Print info messages
  const std::string basename =
      TString::Format("IVcurveBB_%s_%s", hicResult->GetName().c_str(), m_config->GetfNameSuffix())
          .Data();

  std::string rootfilename, pdffilename;
  if (m_config->GetUseDataPath()) {
    rootfilename = hicResult->GetOutputPath() + "/" + basename + ".root";
    pdffilename  = hicResult->GetOutputPath() + "/" + basename + ".pdf";
  }
  else {
    rootfilename = basename + ".root";
    pdffilename  = basename + ".pdf";
  }

  TCanvas c;
  c.cd();
  c.Print((pdffilename + "[").c_str()); // Just open the file

  TFile *rootfile = TFile::Open(rootfilename.c_str(), "RECREATE");

  const std::string hname = TString::Format("iv_%s", hicResult->GetName().c_str()).Data();
  const std::string htitle =
      TString::Format("Back Bias IV for %s", hicResult->GetName().c_str()).Data();

  const int iMax = m_ivPoints;

  TH1F *ivbb = new TH1F(hname.c_str(), htitle.c_str(), iMax, 0., iMax * 100.);

  for (int i = 0; i < iMax; i++)
    ivbb->Fill(i * 100 + 50, hicResult->ibias[i]); // Fill at bin mid point

  ivbb->SetStats(kFALSE);
  ivbb->GetXaxis()->SetTitle("V (mV)");
  ivbb->GetYaxis()->SetTitle("I (mA)");
  ivbb->Draw();
  c.Update();
  c.Print(pdffilename.c_str());

  c.Print((pdffilename + "]").c_str()); // Just close the file

  ivbb->Write();
  rootfile->Close();

  hicResult->SetHasPDF(true);
  hicResult->SetPDFPath(pdffilename);
#endif // HAS_ROOT
}

void TFastPowerAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  TFastPowerTest *powerTest = (TFastPowerTest *)m_scan;

  std::map<std::string, THicCurrents> currents = powerTest->GetCurrents();

  std::map<std::string, THicCurrents>::iterator it, itResult;

  for (it = currents.begin(); it != currents.end(); ++it) {
    TFastPowerResultHic *hicResult   = (TFastPowerResultHic *)m_result->GetHicResult(it->first);
    THicCurrents         hicCurrents = it->second;

    // Copy currents from currents to result, apply cuts, write to file
    hicResult->trip         = hicCurrents.trip;
    hicResult->tripBB       = hicCurrents.tripBB;
    hicResult->iddaSwitchon = hicCurrents.iddaSwitchon;
    hicResult->idddSwitchon = hicCurrents.idddSwitchon;
    hicResult->ibias0       = hicCurrents.ibias0;
    hicResult->ibias3       = hicCurrents.ibias3;
    hicResult->maxBias      = hicCurrents.maxBias;

    for (int i = 0; i < m_ivPoints; i++) {
      hicResult->ibias[i] = hicCurrents.ibias[i];
    }
    hicResult->m_class = GetClassification(hicCurrents, hicResult);
    PrintHicClassification(hicResult);
    hicResult->SetValidity(true);

    CreateIVHisto(hicResult);
  }
  WriteResult();
  m_finished = true;
}

THicClassification TFastPowerAnalysis::GetClassification(THicCurrents         currents,
                                                         TFastPowerResultHic *result)
{
  if (currents.trip) {
    std::cout << "Fast power analysis: HIC classified red due to trip" << std::endl;
    return CLASS_RED;
  }
  if (currents.hicType == HIC_OB)
    return GetClassificationOB(currents, result);
  else {
    std::cout << "Error: test not foreseen for IB HICs" << std::endl;
    return CLASS_UNTESTED;
  }
}

THicClassification TFastPowerAnalysis::GetClassificationOB(THicCurrents         currents,
                                                           TFastPowerResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  if (currents.trip) returnValue = CLASS_RED;

  DoCut(returnValue, CLASS_RED, currents.iddaSwitchon * 1000, "MINIDDA_OB", result, true);
  DoCut(returnValue, CLASS_RED, currents.idddSwitchon * 1000, "MINIDDD_OB", result, true);

  DoCut(returnValue, CLASS_SILVER, currents.idddSwitchon * 1000, "MAXIDDDSILVEROB", result);

  DoCut(returnValue, CLASS_RED, currents.iddaSwitchon * 1000, "MAXIDDA_OB", result);
  DoCut(returnValue, CLASS_RED, currents.idddSwitchon * 1000, "MAXIDDD_OB", result);

  // check for absolute value at 3V and for margin from breakthrough
  if (!currents.tripBB)
    DoCut(returnValue, CLASS_SILVER, currents.ibias[30], "MAXBIAS_3V_IB", result);

  if (currents.tripBB) {
    if (returnValue == CLASS_GOLD) returnValue = CLASS_GOLD_NOBB;
    if (returnValue == CLASS_SILVER) returnValue = CLASS_SILVER_NOBB;
    if (returnValue == CLASS_BRONZE) returnValue = CLASS_BRONZE_NOBB;
  }

  return returnValue;
  // TODO: Add orange for back bias
}

void TFastPowerAnalysis::WriteIVCurve(THic *hic)
{
  char                 fName[200];
  TFastPowerResultHic *result = (TFastPowerResultHic *)m_result->GetHicResult(hic->GetDbId());

  if (m_config->GetUseDataPath()) {
    sprintf(fName, "%s/IVCurve_%s.dat", result->GetOutputPath().c_str(),
            m_config->GetfNameSuffix());
  }
  else {
    sprintf(fName, "IVCurve_%s_%s.dat", hic->GetDbId().c_str(), m_config->GetfNameSuffix());
  }

  result->SetIVFile(fName);

  FILE *fp = fopen(fName, "w");

  for (int i = 0; i < m_ivPoints; i++) {
    fprintf(fp, "%.3f %.1f\n", (float)i / 10, result->ibias[i]);
  }
  fclose(fp);
}

void TFastPowerAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    WriteIVCurve(m_hics.at(ihic));
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/PowerTestResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "PowerTestResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
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

// void TFastPowerResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // string      fileName, ivName;
  // std::size_t slash;
  // DbAddParameter(db, activity, string("IDDD"), idddSwitchon, GetParameterFile());
  // DbAddParameter(db, activity, string("IDDA"), iddaSwitchon, GetParameterFile());
  // DbAddParameter(db, activity, string("Back bias current 0V"), ibias0, GetParameterFile());
  // DbAddParameter(db, activity, string("Back bias current 3V"), ibias3, GetParameterFile());
  // DbAddParameter(db, activity, string("Maximum bias voltage"), maxBias, GetParameterFile());

  // slash    = string(m_resultFile).find_last_of("/");
  // fileName = string(m_resultFile).substr(slash + 1); // strip path

  // slash  = string(m_ivFile).find_last_of("/");
  // ivName = string(m_ivFile).substr(slash + 1); // strip path

  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), fileName);
  // DbAddAttachment(db, activity, attachResult, string(m_ivFile), ivName);
// }

void TFastPowerResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result: \n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "trip:             %d\n\n", trip ? 1 : 0);
  fprintf(fp, "Back bias trip:    %d\n\n", tripBB ? 1 : 0);
  if (tripBB) {
    fprintf(fp, "   max. back bias: %.1f\n\n", maxBias);
  }

  fprintf(fp, "IDDD at switchon: %.3f\n", idddSwitchon);
  fprintf(fp, "IDDA at switchon: %.3f\n", iddaSwitchon);

  fprintf(fp, "IBias at 0V:      %.3f\n", ibias0);
  fprintf(fp, "IBias at 3V:      %.3f\n", ibias3);

  fprintf(fp, "\nI-V-curve file:   %s\n", m_ivFile);
}

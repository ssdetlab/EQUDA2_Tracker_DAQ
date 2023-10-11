#include "TDCTRLAnalysis.h"
#include "AlpideConfig.h"
// #include "DBHelpers.h"
#include "TDCTRLMeasurement.h"
#include "TFifoTest.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

TDctrlAnalysis::TDctrlAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                               TScanConfig *aScanConfig, std::vector<THic *> hics,
                               std::mutex *aMutex, TDctrlResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TDctrlResult();
  FillVariableList();
  m_prediction = new TDctrlResult();
}

void TDctrlAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
  CreatePrediction();
  TTestType testType;
  testType = m_config->GetTestType();
  if (testType != OBHalfStaveOLFAST && testType != OBHalfStaveMLFAST && testType != OBStaveOLFAST &&
      testType != OBStaveMLFAST) {
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      CalculatePrediction(m_hics.at(ihic)->GetDbId());
    }
  }
}

// variables to be displayed in the GUI
void TDctrlAnalysis::FillVariableList() {}

string TDctrlAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Qualification Test");
  case OBHalfStaveOL:
    return string("OB HIC Reception Test");
  case OBHalfStaveML:
    return string("OB HIC Reception Test");
  case IBStave:
    return string("IB HIC Qualification Test");
  case IBStaveEndurance:
    return string("IB Stave Qualification Test");
  case StaveReceptionOL:
    return string("OL HS Qualification Test");
  case StaveReceptionML:
    return string("ML HS Qualification Test");
  default:
    return string("");
  }
}

void TDctrlAnalysis::InitCounters()
{
  // m_counters.clear();


  std::map<std::string, TScanResultHic *>::iterator it;
  std::map<int, TScanResultChip *>::iterator        itChip;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TDctrlResultHic *result = (TDctrlResultHic *)it->second;
    for (itChip = result->m_chipResults.begin(); itChip != result->m_chipResults.end(); ++itChip) {
      TDctrlResultChip *resultChip = (TDctrlResultChip *)itChip->second;
      (void)resultChip; // TODO: Set here the initialisations in the chip (and hic) result if
                        // necessary
      resultChip->slave = false;
    }
  }
}


void TDctrlAnalysis::CalculatePrediction(std::string hicName)
{
  // std::vector<ActivityDB::activityLong> activities;
  // TDctrlResultHic *                     prediction;
  // try {
    // prediction = (TDctrlResultHic *)m_prediction->GetHicResult(hicName);
  // }
  // catch (...) {
    // std::cout << "Error: prediction not found for hic " << hicName << std::endl;
    // return;
  // }

  // prediction->SetValidity(FillPreviousActivities(hicName, &activities));
  // if (!prediction->IsValid()) return;

  // // do the calculation here
  // for (unsigned int i = 0; i < activities.size(); i++) {
    // float value;
    // if (GetPreviousParamValue("DCTRL worst slope", "", activities.at(i), value)) {
      // prediction->worst_slope = value;
    // }
    // if (GetPreviousParamValue("DCTRL worst chi square", "", activities.at(i), value)) {
      // prediction->worst_chisq = value;
    // }
  // }
}


bool TDctrlAnalysis::ChipIsSlave(common::TChipIndex idx)
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    if (m_hics.at(i)->ContainsChip(idx)) {
      if (m_hics.at(i)->GetHicType() == HIC_OB) {
        return ((idx.chipId & 0x7) != 0);
      }
      else {
        return false;
      }
    }
  }
  // this should never happen, but as slave is equivalent to not having a control interface
  // return true for chips which do not belong to any HIC
  return true;
}


void TDctrlAnalysis::WriteResult()
{
  char fName[200];

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    if (!m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->IsValid()) continue;
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/DctrlScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "DctrlScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
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

void TDctrlAnalysis::Fit(std::vector<float> x, std::vector<float> y, float &m, float &b,
                         float &corr, float &chisq)
{
  std::vector<float> dx, dy;
  dx.clear();
  dy.clear();

  chisq = 0;

  float n    = (float)x.size();
  float s_x  = std::accumulate(x.begin(), x.end(), 0.0);
  float s_y  = std::accumulate(y.begin(), y.end(), 0.0);
  float s_xx = std::inner_product(x.begin(), x.end(), x.begin(), 0.0);
  float s_xy = std::inner_product(x.begin(), x.end(), y.begin(), 0.0);

  m = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
  b = (s_y * s_xx - s_x * s_xy) / (n * s_xx - s_x * s_x);

  float av_x = s_x / n;
  float av_y = s_y / n;

  for (unsigned int i = 0; i < x.size(); i++) {
    dx.push_back(x.at(i) - av_x);
    dy.push_back(y.at(i) - av_y);
    float exp = m * x.at(i) + b;
    chisq += pow((y.at(i) - exp), 2) / exp;
  }
  float s_dxdy = std::inner_product(dx.begin(), dx.end(), dy.begin(), 0.0);
  float s_dxdx = std::inner_product(dx.begin(), dx.end(), dx.begin(), 0.0);
  float s_dydy = std::inner_product(dy.begin(), dy.end(), dy.begin(), 0.0);

  corr = s_dxdy / (sqrt(s_dxdx * s_dydy));
}


void TDctrlAnalysis::AnalyseHisto(TScanHisto *histo)
{
  std::vector<float> amp_pos, amp_neg, driver;
  float              m_pos, m_neg, b_pos, b_neg, corr_pos, corr_neg, chisq_neg, chisq_pos;
  char               fName[200];

  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    // add here the analysis of the single histos
    // access values like:
    float maxAmp_n = 0, maxAmp_p = 0, maxRise_p = 0, maxRise_n = 0, maxFall_p = 0, maxFall_n = 0;
    amp_pos.clear();
    amp_neg.clear();
    driver.clear();

    TDctrlResultChip *chipResult =
        (TDctrlResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    TDctrlResultHic *hicResult = (TDctrlResultHic *)FindHicResultForChip(m_chipList.at(ichip));
    if (!hicResult) throw std::runtime_error("HIC result not found, unable to set filename\n");

    if (m_config->GetUseDataPath() && hicResult) {
      sprintf(fName, "%s/DCtrlMeasurement_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "DCtrlMeasurement_%s.dat", m_config->GetfNameSuffix());
    }

    FILE *fp = fopen(fName, "a");
    hicResult->SetScanFile(fName);

    if (ChipIsSlave(m_chipList.at(ichip))) {
      if (chipResult) chipResult->slave = true;
      continue;
    }

    // skip the first one (HIC) or two (HS) points of the measurement
    // measurement values not reliable due to low amplitude
    int skipPoints = m_config->IsHalfStave() ? 2 : 1;

    // loop over all driver settings, read data from histogram
    for (int i = 0; i < 16; i++) {
      float peak_p = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::peak_p));
      float peak_n = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::peak_n));
      float amp_p  = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::amp_p));
      float amp_n  = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::amp_n));
      float rtim_p = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::rtim_p));
      float rtim_n = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::rtim_n));
      float ftim_p = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::ftim_p));
      float ftim_n = ((*histo)(m_chipList.at(ichip), i, TDctrlMeasurement::ftim_n));

      fprintf(fp, "%d %d %f %f %f %f %e %e %e %e\n", m_chipList.at(ichip).chipId & 0xf, i, peak_p,
              peak_n, amp_p, amp_n, rtim_p, rtim_n, ftim_p, ftim_n);

      if (i < skipPoints || amp_p > 10 || amp_n > 10) continue;
      driver.push_back((float)i);
      amp_pos.push_back(amp_p);
      amp_neg.push_back(amp_n);

      if (amp_p < 10 && amp_p > maxAmp_p) maxAmp_p = amp_p;
      if (rtim_p < 10 && rtim_p > maxRise_p) maxRise_p = rtim_p;
      if (ftim_p < 10 && ftim_p > maxFall_p) maxFall_p = ftim_p;
      if (amp_n < 10 && amp_n > maxAmp_n) maxAmp_n = amp_n;
      if (rtim_n < 10 && rtim_n > maxRise_n) maxRise_n = rtim_n;
      if (ftim_n < 10 && ftim_n > maxFall_n) maxFall_n = ftim_n;
    }

    // do the fit for positive and negative polarity
    Fit(driver, amp_pos, m_pos, b_pos, corr_pos, chisq_pos);
    Fit(driver, amp_neg, m_neg, b_neg, corr_neg, chisq_neg);

    // fill chip result
    if (!chipResult) {
      std::cout << "WARNING (DCtrlAnalysis): chipResult = 0" << std::endl;
    }
    else {
      chipResult->m_pos       = m_pos;
      chipResult->b_pos       = b_pos;
      chipResult->chisq_pos   = chisq_pos;
      chipResult->corr_pos    = corr_pos;
      chipResult->maxAmp_pos  = maxAmp_p;
      chipResult->maxRise_pos = maxRise_p;
      chipResult->maxFall_pos = maxFall_p;
      chipResult->m_neg       = m_neg;
      chipResult->b_neg       = b_neg;
      chipResult->chisq_neg   = chisq_neg;
      chipResult->corr_neg    = corr_neg;
      chipResult->maxAmp_neg  = maxAmp_n;
      chipResult->maxRise_neg = maxRise_n;
      chipResult->maxFall_neg = maxFall_n;
    }
    fclose(fp);
  }
}


float TDctrlAnalysis::Max(float a, float b, float c) { return max(max(a, b), c); }


float TDctrlAnalysis::Min(float a, float b, float c) { return min(min(a, b), c); }


void TDctrlAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
    TDctrlResultChip *chipResult =
        (TDctrlResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
    if (!chipResult) {
      std::cout << "WARNING: chipResult = 0" << std::endl;
      continue;
    }
    if (chipResult->slave) continue;
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TDctrlResultHic *hicResult =
          (TDctrlResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
      if (!hicResult) throw std::runtime_error("HIC result not found, unable to write result");

      hicResult->worst_slope = Min(hicResult->worst_slope, chipResult->m_pos, chipResult->m_neg);
      hicResult->worst_maxAmp =
          Min(hicResult->worst_maxAmp, chipResult->maxAmp_pos, chipResult->maxAmp_neg);
      hicResult->worst_chisq =
          Max(hicResult->worst_chisq, chipResult->chisq_pos, chipResult->chisq_neg);
      hicResult->worst_corr =
          Min(hicResult->worst_corr, chipResult->corr_pos, chipResult->corr_neg);
      hicResult->worst_rise =
          Max(hicResult->worst_rise, chipResult->maxRise_pos, chipResult->maxRise_neg);
      hicResult->worst_fall =
          Max(hicResult->worst_fall, chipResult->maxFall_pos, chipResult->maxFall_neg);

      if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
        hicResult->m_class = GetClassificationOB(hicResult);
      }
      else {
        hicResult->m_class = GetClassificationIB(hicResult);
      }
      hicResult->SetValidity(true);
      PrintHicClassification(hicResult);
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    ComparePrediction(m_hics.at(ihic)->GetDbId());
  }

  WriteResult();

  m_finished = true;
}

THicClassification TDctrlAnalysis::GetClassificationIB(TDctrlResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;
  DoCut(returnValue, CLASS_RED, result->worst_maxAmp * 1000, "DCTRLMINAMPIB", result, true);
  DoCut(returnValue, CLASS_RED, result->worst_slope * 1000, "DCTRLMINSLOPEIB", result, true);
  DoCut(returnValue, CLASS_RED, result->worst_chisq * 100, "DCTRLMAXCHISQSILVER", result);
  DoCut(returnValue, CLASS_SILVER, result->worst_rise * 1e9, "DCTRLMAXRISEGREENIB", result);
  DoCut(returnValue, CLASS_SILVER, result->worst_fall * 1e9, "DCTRLMAXFALLGREENIB", result);
  return returnValue;
}

THicClassification TDctrlAnalysis::GetClassificationOB(TDctrlResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;
  if (m_config->IsHalfStave()) {
    DoCut(returnValue, CLASS_RED, result->worst_maxAmp * 1000, "DCTRLMINAMPOBSTAVE", result, true);
    DoCut(returnValue, CLASS_RED, result->worst_slope * 1000, "DCTRLMINSLOPEOBSTAVE", result, true);
  }
  else {
    DoCut(returnValue, CLASS_RED, result->worst_maxAmp * 1000, "DCTRLMINAMPOB", result, true);
    DoCut(returnValue, CLASS_RED, result->worst_slope * 1000, "DCTRLMINSLOPEOB", result, true);
  }
  DoCut(returnValue, CLASS_RED, result->worst_chisq * 100, "DCTRLMAXCHISQSILVER", result);
  DoCut(returnValue, CLASS_SILVER, result->worst_rise * 1e9, "DCTRLMAXRISEGREENOB", result);
  DoCut(returnValue, CLASS_SILVER, result->worst_fall * 1e9, "DCTRLMAXFALLGREENOB", result);
  return returnValue;
}

void TDctrlResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  fprintf(fp, "Worst maximum amplitude: %f\n", worst_maxAmp);
  fprintf(fp, "Worst slope:             %f\n", worst_slope);
  fprintf(fp, "    ratio to previous:   %f\n", worst_slopeRatio);
  fprintf(fp, "Worst chi square:        %f\n", worst_chisq);
  fprintf(fp, "    ratio to previous:   %f\n", worst_chisqRatio);
  fprintf(fp, "Worst correlation:       %f\n", worst_corr);
  fprintf(fp, "Worst rise time:         %e\n", worst_rise);
  fprintf(fp, "Worst fall time:         %e\n", worst_fall);

  fprintf(fp, "\nScan file: %s\n", m_scanFile);

  std::map<int, TScanResultChip *>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
}

// void TDctrlResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string fileName, scanName;
  // std::size_t slash;

  // // to be updated, probably divide according to tested device (IB / OB HIC)
  // DbAddParameter(db, activity, string("DCTRL worst max amplitude"), worst_maxAmp,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst slope"), worst_slope, GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst chi square"), worst_chisq, GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst correlation"), worst_corr, GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst rise time"), worst_rise, GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst fall time"), worst_fall, GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst chi square ratio"), worst_chisqRatio,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("DCTRL worst slope ratio"), worst_slopeRatio,
                //  GetParameterFile());

  // slash    = string(m_resultFile).find_last_of("/");
  // fileName = string(m_resultFile).substr(slash + 1); // strip path
  // slash    = string(m_scanFile).find_last_of("/");
  // scanName = string(m_scanFile).substr(slash + 1); // strip path

  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), fileName);
  // DbAddAttachment(db, activity, attachResult, string(m_scanFile), scanName);
// }


void TDctrlResultHic::Compare(TScanResultHic *aPrediction)
{
  TDctrlResultHic *prediction = (TDctrlResultHic *)aPrediction;
  if (prediction->worst_slope > 0)
    worst_slopeRatio = worst_slope / prediction->worst_slope;
  else
    worst_slopeRatio = 0;
  if (prediction->worst_chisq > 0)
    worst_chisqRatio = worst_chisq / prediction->worst_chisq;
  else
    worst_chisqRatio = 0;
}


float TDctrlResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TDctrlResultChip::WriteToFile(FILE *fp)
{
  if (slave) return;
  fprintf(fp, "Fit positive polarity:\n");
  fprintf(fp, "Slope p: %f\n", m_pos);
  fprintf(fp, "Intercept p: %f\n", b_pos);
  fprintf(fp, "Chi sq p: %f\n", chisq_pos);
  fprintf(fp, "Correlation coeff p: %f\n", corr_pos);

  fprintf(fp, "\nMax values positive polarity:\n");
  fprintf(fp, "Max. amplitude p: %f\n", maxAmp_pos);
  fprintf(fp, "Max. rise time p: %e\n", maxRise_pos);
  fprintf(fp, "Max. fall time p: %e\n", maxFall_pos);

  fprintf(fp, "\nFit negative polarity:\n");
  fprintf(fp, "Slope n: %f\n", m_neg);
  fprintf(fp, "Intercept n: %f\n", b_neg);
  fprintf(fp, "Chi sq n: %f\n", chisq_neg);
  fprintf(fp, "Correlation coeff n: %f\n", corr_neg);

  fprintf(fp, "\nMax values negative polarity:\n");
  fprintf(fp, "Max. amplitude n: %f\n", maxAmp_neg);
  fprintf(fp, "Max. rise time n: %e\n", maxRise_neg);
  fprintf(fp, "Max. fall time n: %e\n", maxFall_neg);
}

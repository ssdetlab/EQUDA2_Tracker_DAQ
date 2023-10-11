#include "TSCurveAnalysis.h"
// #include "DBHelpers.h"
#include <iostream>
#include <string>
#include <vector>

#include <TMath.h>

double ErrorFunc2(double *x, double *par)
{
  // double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  double y = par[2] * (1 + TMath::Erf((x[0] - par[0]) / (sqrt(2) * par[1])));
  return y;
}

TSCurveAnalysis::TSCurveAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                                 TScanConfig *aScanConfig, std::vector<THic *> hics,
                                 std::mutex *aMutex, TSCurveResult *aResult, float resultFactor)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  m_nPulseInj           = m_config->GetNInj();
  m_resultFactor        = resultFactor;
  m_writeRawData        = m_config->GetParamValue("RAWDATA");
  m_writeNoHitPixels    = false;
  m_writeNoThreshPixels = false;
  m_writeStuckPixels    = false;
  m_writeFitResults     = true;
  m_fDoFit              = true;
  m_targetThreshold     = m_config->GetParamValue("TARGETTHRESH");
  m_nominal             = ((TSCurveScan *)m_scan)->GetNominal();

  m_speedy = (m_config->GetParamValue("SPEEDY") != 0);
  if (IsThresholdScan()) {
    m_startPulseAmplitude = m_config->GetChargeStart();
    m_stopPulseAmplitude  = m_config->GetChargeStop();
    m_stepPulseAmplitude  = m_config->GetChargeStep();
  }
  else if (IsVCASNTuning()) {
    m_startPulseAmplitude = m_config->GetVcasnStart();
    m_stopPulseAmplitude  = m_config->GetVcasnStop();
    m_stepPulseAmplitude  = m_config->GetVcasnStep();
  }
  else {
    m_startPulseAmplitude = m_config->GetIthrStart();
    m_stopPulseAmplitude  = m_config->GetIthrStop();
    m_stepPulseAmplitude  = m_config->GetIthrStep();
  }

  if (aResult)
    m_result = aResult;
  else
    m_result = new TSCurveResult();
  m_prediction = new TSCurveResult();
  FillVariableList();
}

void TSCurveAnalysis::FillVariableList()
{
  if (IsThresholdScan()) {
    m_variableList.insert(std::pair<const char *, TResultVariable>("Dead Pixels", deadPix));
    m_variableList.insert(
        std::pair<const char *, TResultVariable>("Pixels without threshold", noThreshPix));
    m_variableList.insert(std::pair<const char *, TResultVariable>("Hot Pixels", hotPix));
    m_variableList.insert(std::pair<const char *, TResultVariable>("av. Threshold", thresh));
    m_variableList.insert(std::pair<const char *, TResultVariable>("Threshold RMS", threshRms));
    m_variableList.insert(std::pair<const char *, TResultVariable>("av. Noise", noise));
    m_variableList.insert(std::pair<const char *, TResultVariable>("Noise RMS", noiseRms));
  }
  else if (IsVCASNTuning()) {
    m_variableList.insert(std::pair<const char *, TResultVariable>("av. VCASN", vcasn));
  }
  else {
    m_variableList.insert(std::pair<const char *, TResultVariable>("av. ITHR", ithr));
  }
}

string TSCurveAnalysis::GetPreviousTestType()
{
  switch (m_config->GetTestType()) {
  case OBQualification:
    if (((TSCurveParameters *)m_scan->GetParameters())->backBias == 0.)
      return string("ALPIDEB Chip Testing Analysis");
    else
      return string("");
  case OBEndurance:
    return string("OB HIC Qualification Test");
  case OBReception:
    return string("OB HIC Qualification Test");
  case OBHalfStaveOL:
    return string("OB HIC Qualification Test");
  case OBHalfStaveML:
    return string("OB HIC Qualification Test");
  case IBQualification:
    if (((TSCurveParameters *)m_scan->GetParameters())->backBias == 0.)
      return string("ALPIDEB Chip Testing Analysis");
    else
      return string("");
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

bool TSCurveAnalysis::CheckPixelNoHits(TGraph *aGraph)
{

  for (int itrPoint = 0; itrPoint < aGraph->GetN(); itrPoint++) {
    double x = 0;
    double y = 0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y != 0) {
      return false;
    }
  }

  return true;
}

bool TSCurveAnalysis::CheckPixelHot(TGraph *aGraph)
{
  for (int itrPoint = 0; itrPoint < aGraph->GetN(); itrPoint++) {
    double x = 0;
    double y = 0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y < 0.5 * m_nPulseInj) {
      return false;
    }
  }

  return true;
}

void TSCurveAnalysis::PrepareFiles()
{
  char fName[200];
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TSCurveResultChip *chipResult = (TSCurveResultChip *)m_result->GetChipResult(m_chipList.at(i));
    if (m_writeRawData) {
      if (m_config->GetUseDataPath()) {
        sprintf(fName, "%s/Threshold_RawData_%s_Chip%d.dat", chipResult->GetOutputPath().c_str(),
                m_config->GetfNameSuffix(), m_chipList.at(i).chipId);
      }
      else {
        sprintf(fName, "Threshold_RawData_%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(),
                m_chipList.at(i).boardIndex, m_chipList.at(i).dataReceiver,
                m_chipList.at(i).chipId);
      }
      chipResult->SetRawFile(fName);
      chipResult->m_rawFP = fopen(fName, "w");
    }

    if (m_writeFitResults) {
      if (m_config->GetUseDataPath()) {
        sprintf(fName, "%s/Threshold_FitResults_%s_Chip%d.dat", chipResult->GetOutputPath().c_str(),
                m_config->GetfNameSuffix(), m_chipList.at(i).chipId);
      }
      else {
        sprintf(fName, "Threshold_FitResults__%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(),
                m_chipList.at(i).boardIndex, m_chipList.at(i).dataReceiver,
                m_chipList.at(i).chipId);
      }
      chipResult->SetFitFile(fName);
      chipResult->m_fitFP = fopen(fName, "w");
    }
  }
}

void TSCurveAnalysis::Initialize()
{
  ReadChipList();
  CreateHicResults();
  CreatePrediction();
  PrepareFiles();
  TTestType testType;
  testType = m_config->GetTestType();
  if (testType != OBHalfStaveOLFAST && testType != OBHalfStaveMLFAST && testType != OBStaveOLFAST &&
      testType != OBStaveMLFAST) {
    if (IsThresholdScan() && (!m_nominal)) { // do only for threshold scan after tuning
      for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
        CalculatePrediction(m_hics.at(ihic)->GetDbId());
      }
    }
  }
}

// TODO: set file names here???
void TSCurveAnalysis::InitCounters()
{
  std::map<std::string, TScanResultHic *>::iterator it;

  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    TSCurveResultHic *result = (TSCurveResultHic *)it->second;
    result->m_backBias       = m_scan->GetBackBias();
    result->m_nominal        = ((TSCurveScan *)m_scan)->GetNominal();
    result->m_VCASNTuning    = IsVCASNTuning();
    result->m_ITHRTuning     = IsITHRTuning();
    result->m_thresholdScan  = IsThresholdScan();
  }
}

void TSCurveAnalysis::CalculatePrediction(std::string hicName)
{
  // std::vector<ActivityDB::activityLong> activities;
  // TSCurveResultHic *                    prediction;
  // try {
    // prediction = (TSCurveResultHic *)m_prediction->GetHicResult(hicName);
  // }
  // catch (...) {
    // std::cout << "Error: prediction not found for hic " << hicName << std::endl;
    // return;
  // }

  // prediction->SetValidity(FillPreviousActivities(hicName, &activities));
  // if (!prediction->IsValid()) return;

  // char nameDeadPixels[50], nameNoThresh[50];
  // sprintf(nameDeadPixels, "Dead pixels threshold tuned %dV",
          // (int)(((TSCurveParameters *)m_scan->GetParameters())->backBias));
  // sprintf(nameNoThresh, "Pixels without threshold tuned %dV",
          // (int)(((TSCurveParameters *)m_scan->GetParameters())->backBias));
  // // do the calculation here
  // for (unsigned int i = 0; i < activities.size(); i++) {
    // float value;
    // if (GetPreviousParamValue(nameDeadPixels, "Dead Pixels", activities.at(i), value)) {
      // prediction->m_nDead += value;
    // }
    // if (GetPreviousParamValue(nameNoThresh, "Threshold Pixels", activities.at(i), value)) {
      // if (GetPreviousTestType() == "ALPIDEB Chip Testing Analysis") {
        // prediction->m_nNoThresh += (512 * 1024 - value);
      // }
      // else {
        // prediction->m_nNoThresh += value;
      // }
    // }
  // }
}

// in some cases (VCASN tuning with back bias) the number of hits drops again after
// reaching the plateau, which confuses the root fit. In order to avoid this, fill
// the graph with 100% values, once the plateau has been reached.
void TSCurveAnalysis::FillGraph(TGraph *aGraph)
{
  int      nMin          = m_nPulseInj - 1;
  int      count         = 0;
  int      minForPlateau = 3;
  bool     plateau       = false;
  Double_t x, y;

  for (int i = 0; i < aGraph->GetN(); i++) {
    aGraph->GetPoint(i, x, y);
    if (!plateau) { // search for N consecutive points above nMin
      if (y > nMin)
        count++;
      else
        count = 0;
      if (count == minForPlateau) {
        plateau = true;
      }
    }
    else {
      aGraph->SetPoint(i, x, m_nPulseInj);
    }
  }
  //        gPixel->SetPoint(gPixel->GetN(), iPulse * m_resultFactor, entries);
}

// TODO: Write Raw Data, write fit data
void TSCurveAnalysis::AnalyseHisto(TScanHisto *histo)
{
  int row                   = histo->GetIndex();
  int vcasnTuningOutOfRange = 0;
  for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip++) {
    TSCurveResultChip *chipResult =
        (TSCurveResultChip *)m_result->GetChipResult(m_chipList.at(iChip));
    for (int iCol = 0; iCol < common::nCols; iCol++) {
      TGraph *gPixel = new TGraph();
      for (int iPulse = m_startPulseAmplitude; iPulse < m_stopPulseAmplitude; iPulse++) {
        int entries = (int)(*histo)(m_chipList.at(iChip), iCol, iPulse - m_startPulseAmplitude);
        gPixel->SetPoint(gPixel->GetN(), iPulse * m_resultFactor, entries);
        if (m_writeRawData) {
          fprintf(chipResult->m_rawFP, "%d %d %d %d\n", iCol, row, iPulse, entries);
        }
      }
      if (gPixel->GetN() == 0) {
        delete gPixel;
        continue;
      }
      if (CheckPixelNoHits(gPixel)) {
        chipResult->m_nDead++;
      }
      else if (CheckPixelHot(gPixel)) {
        chipResult->m_nHot++;
      }
      else {
        if (IsVCASNTuning()) FillGraph(gPixel);
        common::TErrFuncFitResult fitResult = DoFit(gPixel, m_speedy);
        if (fitResult.threshold == 0) {
          chipResult->m_nNoThresh++;
        }
        else if (IsVCASNTuning() && fitResult.noise < 0) {
          ++vcasnTuningOutOfRange;
          std::cout << "Vcasn tuning out-of-range in pixel " << iCol << "/" << row << " of chip ID "
                    << m_chipList.at(iChip).chipId << ", receiver "
                    << m_chipList.at(iChip).dataReceiver << ", board "
                    << m_chipList.at(iChip).boardIndex << "." << std::endl;
        }
        else {
          chipResult->m_thresholdAv += fitResult.threshold;
          chipResult->m_noiseAv += fitResult.noise;
          chipResult->m_noiseSq += pow(fitResult.noise, 2);
          chipResult->m_threshSq += pow(fitResult.threshold, 2);
          chipResult->m_nEntries++;
          for (unsigned int iHic = 0; iHic < m_hics.size(); iHic++) {
            if (!(m_hics.at(iHic)->ContainsChip(m_chipList.at(iChip)))) continue;
            TSCurveResultHic *hicResult =
                (TSCurveResultHic *)m_result->GetHicResults()->at(m_hics.at(iHic)->GetDbId());
            hicResult->m_noiseAv += fitResult.noise;
            hicResult->m_noiseSq += pow(fitResult.noise, 2);
            hicResult->m_nEntries++;
          }
        }
        if (m_writeFitResults) {
          fprintf(chipResult->m_fitFP, "%d %d %f %f %f\n", iCol, row, fitResult.threshold,
                  fitResult.noise, fitResult.redChi2);
        }
      }
      delete gPixel;
    }
  }
  if (IsVCASNTuning()) {
    std::cout << vcasnTuningOutOfRange << " pixels out-of-range in the Vcasn tuning." << std::endl;
  }
}

void TSCurveAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  TErrorCounter  errCount = ((TMaskScan *)m_scan)->GetErrorCount();
  TSCurveResult *result   = (TSCurveResult *)m_result;

  result->m_nTimeout  = errCount.nTimeout;
  result->m_n8b10b    = errCount.n8b10b;
  result->m_nOversize = errCount.nOversizeEvent;
  result->m_nCorrupt  = errCount.nCorruptEvent;

  for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip++) {
    TSCurveResultChip *chipResult =
        (TSCurveResultChip *)m_result->GetChipResult(m_chipList.at(iChip));
    chipResult->CalculateAverages();
    if (!m_nominal) {
      chipResult->m_deviation = (chipResult->m_thresholdAv - m_targetThreshold) / m_targetThreshold;
      chipResult->m_deviation *= 100; // value in %
    }
    if (m_writeRawData) fclose(chipResult->m_rawFP);
    if (m_writeFitResults) fclose(chipResult->m_fitFP);
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TSCurveResultHic *hicResult =
        (TSCurveResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());
    hicResult->CalculateAverages();
    for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
      if (!(m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TSCurveResultChip *chipResult =
          (TSCurveResultChip *)m_result->GetChipResult(m_chipList.at(ichip));
      if (chipResult->m_thresholdAv < hicResult->m_minChipAv)
        hicResult->m_minChipAv = chipResult->m_thresholdAv;
      if (chipResult->m_thresholdAv > hicResult->m_maxChipAv)
        hicResult->m_maxChipAv = chipResult->m_thresholdAv;
      if (chipResult->m_noiseAv > hicResult->m_maxChipNoise)
        hicResult->m_maxChipNoise = chipResult->m_noiseAv;
      hicResult->m_nDead += chipResult->m_nDead;
      hicResult->m_nNoThresh += chipResult->m_nNoThresh;
      if (chipResult->m_nDead > hicResult->m_nDeadWorstChip)
        hicResult->m_nDeadWorstChip = chipResult->m_nDead;
      if (chipResult->m_nNoThresh > hicResult->m_nNoThreshWorstChip)
        hicResult->m_nNoThreshWorstChip = chipResult->m_nNoThresh;
      if (chipResult->m_threshRelativeRms > hicResult->m_maxRelativeRms)
        hicResult->m_maxRelativeRms = chipResult->m_threshRelativeRms;
      if (chipResult->m_thresholdRms > hicResult->m_maxRms)
        hicResult->m_maxRms = chipResult->m_thresholdRms;
      if ((!m_nominal) && (fabs(chipResult->m_deviation) > fabs(hicResult->m_maxDeviation)))
        hicResult->m_maxDeviation = chipResult->m_deviation;
      hicResult->m_nHot += chipResult->m_nHot;
    }
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult, m_hics.at(ihic));
    }
    else {
      hicResult->m_class = GetClassificationIB(hicResult, m_hics.at(ihic));
    }
    hicResult->SetValidity(true);
    PrintHicClassification(hicResult);
    if (IsThresholdScan() && (!m_nominal)) { // do only for threshold scan after tuning
      ComparePrediction(m_hics.at(ihic)->GetDbId());
    }
  }
  WriteResult();
  m_finished = true;
}

// TODO: Add readout errors, requires dividing readout errors by hic (receiver)
// TODO: Make two cuts (red and orange)?
THicClassification TSCurveAnalysis::GetClassificationOB(TSCurveResultHic *result, THic *hic)
{
  THicClassification returnValue = CLASS_GOLD;
  if (!IsThresholdScan()) return CLASS_UNTESTED; // for the time being exclude class for tuning
  if (((TSCurveScan *)m_scan)->GetNominal())
    return CLASS_UNTESTED; // classify only tuned thresholds

  for (auto chip : m_chipList) {
    if (!hic->ContainsChip(chip)) continue;
    int                chipId     = chip.chipId;
    TSCurveResultChip *chipResult = (TSCurveResultChip *)result->m_chipResults.at(chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_GOLD",
          result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_SILVER",
          result, false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_BRONZE", result,
          false, chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_GOLD", result, false,
          chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_SILVER", result,
          false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_BRONZE", result, false,
          chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_noiseAv + 0.9, "THRESH_MAXNOISE_OB", result,
          false, chipId);
  }
  return returnValue;
}

THicClassification TSCurveAnalysis::GetClassificationIB(TSCurveResultHic *result, THic *hic)
{
  THicClassification returnValue = CLASS_GOLD;
  if (!IsThresholdScan()) return CLASS_UNTESTED; // for the time being exclude class for tuning
  if (((TSCurveScan *)m_scan)->GetNominal())
    return CLASS_UNTESTED; // classify only tuned thresholds

  for (unsigned int ichip = 0; ichip < hic->GetChips().size(); ichip++) {
    if (!hic->GetChips().at(ichip)->GetConfig()->IsEnabled()) continue;
    int                chipId     = hic->GetChips().at(ichip)->GetConfig()->GetChipId() & 0xf;
    TSCurveResultChip *chipResult = (TSCurveResultChip *)result->m_chipResults.at(chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_GOLD",
          result, false, chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_SILVER",
          result, false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nNoThresh, "THRESH_MAXNOTHRESH_CHIP_BRONZE", result,
          false, chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_GOLD", result, false,
          chipId);
    DoCut(returnValue, CLASS_BRONZE, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_SILVER", result,
          false, chipId);
    DoCut(returnValue, CLASS_RED, chipResult->m_nDead, "THRESH_MAXDEAD_CHIP_BRONZE", result, false,
          chipId);

    DoCut(returnValue, CLASS_SILVER, chipResult->m_noiseAv + 0.9, "THRESH_MAXNOISE_IB", result,
          false, chipId);
  }

  return returnValue;
}

void TSCurveAnalysis::WriteResult()
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    if (m_config->GetUseDataPath()) {
      if (IsThresholdScan())
        sprintf(fName, "%s/ThresholdScanResult_%s.dat", hicResult->GetOutputPath().c_str(),
                m_config->GetfNameSuffix());
      else if (IsVCASNTuning()) {
        sprintf(fName, "%s/VCASNTuneResult_%s.dat", hicResult->GetOutputPath().c_str(),
                m_config->GetfNameSuffix());
      }
      else {
        sprintf(fName, "%s/ITHRTuneResult_%s.dat", hicResult->GetOutputPath().c_str(),
                m_config->GetfNameSuffix());
      }
    }
    else {
      if (IsThresholdScan())
        sprintf(fName, "ThresholdScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
                m_config->GetfNameSuffix());
      else if (IsVCASNTuning()) {
        sprintf(fName, "VCASNTuneResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
                m_config->GetfNameSuffix());
      }
      else {
        sprintf(fName, "ITHRTuneResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
                m_config->GetfNameSuffix());
      }
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

common::TErrFuncFitResult TSCurveAnalysis::DoSpeedyFit(TGraph *aGraph)
{
  common::TErrFuncFitResult Result;
  TGraph *                  diffGraph = new TGraph;

  ddxGraph(aGraph, diffGraph);

  Result.threshold = abs(meanGraph(diffGraph));
  Result.noise     = rmsGraph(diffGraph);
  Result.redChi2   = 0;

  delete diffGraph;
  return Result;
}

common::TErrFuncFitResult TSCurveAnalysis::DoRootFit(TGraph *aGraph)
{
  common::TErrFuncFitResult Result;
  TF1 *                     fitfcn;

  if (IsITHRTuning()) {
    fitfcn = new TF1("fitfcn", ErrorFunc2, m_stopPulseAmplitude * m_resultFactor,
                     m_startPulseAmplitude * m_resultFactor, 4);
  }
  else {
    fitfcn = new TF1("fitfcn", ErrorFunc2, m_startPulseAmplitude * m_resultFactor,
                     m_stopPulseAmplitude * m_resultFactor, 4);
  }
  // Threshold start value
  fitfcn->SetParameter(0, FindStart(aGraph, m_resultFactor, m_nPulseInj));

  // Noise start value
  if (IsThresholdScan()) {
    fitfcn->SetParameter(1, 5);
  }
  else if (IsVCASNTuning()) {
    fitfcn->SetParameter(1, 1);
  }
  else { // ITHR tuning
    fitfcn->SetParameter(1, 8);
  }
  // Amplitude start value
  fitfcn->SetParameter(2, .5 * m_nPulseInj);

  aGraph->Fit("fitfcn", "RQ");

  float threshold = abs(fitfcn->GetParameter(0));

  if ((threshold > abs(m_startPulseAmplitude * m_resultFactor)) &&
      (threshold < abs(m_stopPulseAmplitude * m_resultFactor))) {
    Result.threshold = threshold;
    Result.noise     = fitfcn->GetParameter(1);
    Result.redChi2   = fitfcn->GetChisquare() / fitfcn->GetNDF();
  }
  else { // result is outside scanned range -> usually unreliable values
    Result.threshold = 0;
    Result.noise     = 0;
    Result.redChi2   = 0;
  }

  delete fitfcn;

  return Result;
}

common::TErrFuncFitResult TSCurveAnalysis::DoFit(TGraph *aGraph, bool speedy)
{
  if (speedy) {
    return DoSpeedyFit(aGraph);
  }
  else {
    return DoRootFit(aGraph);
  }
}

////////////////////////////////////////////////////////////////////
//
//     Methods for speedy fit
//          - (find the mean of the derivative (erf->gaussian))
//          - returns 0 if |mean| > 500 (pixel received twice.
//            (ignore these pixels when calculating chip mean)
//
////////////////////////////////////////////////////////////////////

double TSCurveAnalysis::meanGraph(TGraph *resultGraph)
{ // returns the weighted mean x value
  double  sum  = 0.0;
  double  norm = 0.0;
  double *xs   = resultGraph->GetX();
  double *ys   = resultGraph->GetY();

  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum += xs[i] * ys[i]; // xs=value, ys=weight
    norm += ys[i];
  }

  if (norm == 0) { // dead pixel
    return -1;
  }
  if (abs(sum / norm) > 500) { // outliers occur when a pixel is received twice; return 0.
    return 0;
  }
  return sum / norm;
}

double TSCurveAnalysis::rmsGraph(TGraph *resultGraph)
{
  double sum  = 0.0;
  double norm = 0.0;
  double mean = meanGraph(resultGraph);

  if (mean == 0) return 0;
  if (mean == -1) return -1;

  double *xs = resultGraph->GetX();
  double *ys = resultGraph->GetY();

  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum += ys[i] * (xs[i] - mean) * (xs[i] - mean);
    norm += ys[i];
  }

  if (sqrt(abs(sum / norm)) > 500) {
    return 0;
  }
  return sqrt(abs(sum / norm));
}

void TSCurveAnalysis::ddxGraph(TGraph *aGraph, TGraph *resultGraph)
{ // resultGraph contains the derivative of aGraph wrt x (1st order)
  // Results are at MIDPOINTS of the old graph!
  double *xs = aGraph->GetX(); // all x-coords
  double *ys = aGraph->GetY();

  for (int i = 0; i < aGraph->GetN() - 1; i++) {
    // xval=avg of x1 and x2
    if (xs[i + 1] == xs[i]) std::cout << "ERROR: repeated xval" << std::endl;
    resultGraph->SetPoint(resultGraph->GetN(), 0.5 * (xs[i + 1] + xs[i]),
                          (ys[i + 1] - ys[i]) / (xs[i + 1] - xs[i]));
  }
}

////////////////////////////////////////////////////////////////////
//
//     Methods for root fit
//
////////////////////////////////////////////////////////////////////

float TSCurveAnalysis::FindStartStandard(TGraph *aGraph, int nInj)
{
  float   Upper = -1;
  float   Lower = -1;
  double *xs    = aGraph->GetX();
  double *ys    = aGraph->GetY();

  for (int i = 0; i < aGraph->GetN(); i++) {
    if (ys[i] == nInj) {
      Upper = (float)xs[i];
      break;
    }
  }
  if (Upper == -1) return -1;
  for (int i = aGraph->GetN() - 1; i > 0; i--) {
    if (ys[i] == 0) {
      Lower = (float)xs[i];
      break;
    }
  }
  if ((Lower == -1) || (Upper == -1)) {
    return -1;
  }
  if (Upper < Lower) {
    return -1;
  }
  return (Upper + Lower) / 2.0;
}

float TSCurveAnalysis::FindStartInverse(TGraph *aGraph, int nInj)
{
  float   Upper = -1;
  float   Lower = -1;
  double *xs    = aGraph->GetX();
  double *ys    = aGraph->GetY();

  for (int i = aGraph->GetN() - 1; i > -1; i--) {
    if (ys[i] == nInj) {
      Lower = (float)xs[i];
      break;
    }
  }
  if (Lower == -1) return -1;
  for (int i = 0; i < aGraph->GetN(); i++) {
    if (ys[i] == 0) {
      Upper = (float)xs[i];
      break;
    }
  }
  if ((Lower == -1) || (Upper == -1)) {
    return -1;
  }
  if (Upper > Lower) {
    return -1;
  }
  return (Upper + Lower) / 2.0;
}

float TSCurveAnalysis::FindStart(TGraph *aGraph, int resultFactor, int nInj)
{
  if (resultFactor > 0) {
    return FindStartStandard(aGraph, nInj);
  }
  else {
    return FindStartInverse(aGraph, nInj);
  }
}

void TSCurveResultChip::CalculateAverages()
{
  if (m_nEntries > 0) {
    m_thresholdAv /= m_nEntries;
    m_noiseAv /= m_nEntries;
    m_thresholdRms = sqrt(m_threshSq / m_nEntries - pow(m_thresholdAv, 2));
    m_noiseRms     = sqrt(m_noiseSq / m_nEntries - pow(m_noiseAv, 2));
    if (m_thresholdAv > 0) m_threshRelativeRms = m_thresholdRms / m_thresholdAv;
  }
}

void TSCurveResultChip::WriteToFile(FILE *fp)
{
  if (m_analysis->IsThresholdScan()) {
    fprintf(fp, "Pixels without hits:      %d\n", m_nDead);
    fprintf(fp, "Pixels without threshold: %d\n", m_nNoThresh);
    fprintf(fp, "Hot pixels:               %d\n\n", m_nHot);

    fprintf(fp, "Av. Threshold: %.1f\n", m_thresholdAv);
    fprintf(fp, "Threshold RMS: %.1f\n", m_thresholdRms);
    fprintf(fp, "Deviation:     %.2f\n", m_deviation);

    fprintf(fp, "Av. Noise:     %.1f\n", m_noiseAv);
    fprintf(fp, "Noise RMS:     %.1f\n", m_noiseRms);
  }
  else if (m_analysis->IsVCASNTuning()) {
    fprintf(fp, "Av. VCASN: %.1f\n", m_thresholdAv);
    fprintf(fp, "VCASN RMS: %.1f\n", m_thresholdRms);
  }
  else { // ITHR
    fprintf(fp, "Av. ITHR: %.1f\n", m_thresholdAv);
    fprintf(fp, "ITHR RMS: %.1f\n", m_thresholdRms);
  }
}

float TSCurveResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  case vcasn:
    return m_thresholdAv;
  case ithr:
    return m_thresholdAv;
  case thresh:
    return m_thresholdAv;
  case threshRms:
    return m_thresholdRms;
  case noise:
    return m_noiseAv;
  case noiseRms:
    return m_noiseRms;
  case deadPix:
    return m_nDead;
  case noThreshPix:
    return m_nNoThresh;
  case hotPix:
    return m_nHot;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

void TSCurveResultHic::Compare(TScanResultHic *aPrediction)
{
  TSCurveResultHic *prediction = (TSCurveResultHic *)aPrediction;
  m_nDeadIncrease              = m_nDead - prediction->m_nDead;
  m_nNoThreshIncrease          = m_nNoThresh - prediction->m_nNoThresh;
}


void TSCurveResultHic::GetParameterSuffix(std::string &suffix, std::string &file_suffix)
{
  if (m_thresholdScan) {
    if (m_nominal) {
      suffix      = " threshold nominal ";
      file_suffix = "_ThreshNominal";
    }
    else {
      suffix      = " threshold tuned ";
      file_suffix = "_ThreshTuned";
    }
  }
  else if (m_VCASNTuning) {
    suffix      = " VCASN tune ";
    file_suffix = "_VCASNTune";
  }
  else if (m_ITHRTuning) {
    suffix      = " ITHR tune ";
    file_suffix = "_ITHRTune";
  }
  suffix += (std::to_string((int)m_backBias) + std::string("V"));
  file_suffix += (string("_") + std::to_string((int)m_backBias) + std::string("V"));
}


// void TSCurveResultHic::WriteClassToDB(AlpideDB *db, ActivityDB::activity &activity,
                                      // std::string scanName)
// {
  // // for the time being only write class of threshold scan after tuning
  // if (!m_thresholdScan) return;
  // if (m_nominal) return;
  // TScanResultHic::WriteClassToDB(db, activity, scanName);
// }


// void TSCurveResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string suffix, file_suffix, fileName, remoteName;

  // GetParameterSuffix(suffix, file_suffix);

  // if (m_thresholdScan) {
    // DbAddParameter(db, activity, string("Dead pixels") + suffix, (float)m_nDead,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Pixels without") + suffix, (float)m_nNoThresh,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Dead pixels, worst chip,") + suffix,
                  //  (float)m_nDeadWorstChip, GetParameterFile());
    // DbAddParameter(db, activity, string("Pixels without thresh, worst,") + suffix,
                  //  (float)m_nNoThreshWorstChip, GetParameterFile());
    // DbAddParameter(db, activity, string("Average noise") + suffix, (float)m_noiseAv,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Maximum chip noise") + suffix, (float)m_maxChipNoise,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Maximum relative RMS") + suffix, (float)m_maxRelativeRms,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Maximum RMS") + suffix, (float)m_maxRms,
                  //  GetParameterFile());
    // if (!m_nominal) {
      // DbAddParameter(db, activity, string("Maximum threshold deviation,") + suffix,
                    //  (float)m_maxDeviation, GetParameterFile());
      // DbAddParameter(db, activity, string("Dead pixels increase") + suffix, (float)m_nDeadIncrease,
                    //  GetParameterFile());
      // DbAddParameter(db, activity, string("Pixels without thresh increase") + suffix,
                    //  (float)m_nNoThreshIncrease, GetParameterFile());
    // }
  // }
  // DbAddParameter(db, activity, string("Minimum chip avg") + suffix, (float)m_minChipAv,
                //  GetParameterFile());
  // DbAddParameter(db, activity, string("Maximum chip avg") + suffix, (float)m_maxChipAv,
                //  GetParameterFile());

  // std::size_t slash = string(m_resultFile).find_last_of("/");
  // fileName          = string(m_resultFile).substr(slash + 1); // strip path
  // std::size_t point = fileName.find_last_of(".");
  // remoteName        = fileName.substr(0, point) + file_suffix + ".dat";
  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), remoteName);
// }

void TSCurveResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

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

  if (!m_nominal) {
    std::cout << std::endl << "Maximum deviation from target: " << m_maxDeviation << std::endl;
  }
  std::cout << std::endl << "Maximum relative rms:          " << m_maxRelativeRms << std::endl;
  std::cout << std::endl << "Maximum rms:          " << m_maxRms << std::endl;
  std::cout << std::endl << "Dead Pixels:         " << m_nDead << std::endl;
  if (!m_nominal) std::cout << "   Increase:         " << m_nDeadIncrease << std::endl;
  std::cout << "No Threshold Pixels: " << m_nNoThresh << std::endl;
  if (!m_nominal) std::cout << "   Increase:         " << m_nNoThreshIncrease << std::endl;
}

void TSCurveResult::WriteToFileGlobal(FILE *fp)
{
  fprintf(fp, "8b10b errors:\t%d\n", m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n", m_nCorrupt);
  fprintf(fp, "Oversized events:\t%d\n", m_nOversize);
  fprintf(fp, "Timeouts:\t%d\n", m_nTimeout);
}

void TSCurveResultHic::CalculateAverages()
{
  if (m_nEntries > 0) {
    m_noiseAv /= m_nEntries;
    m_noiseRms = sqrt(m_noiseSq / m_nEntries - pow(m_noiseAv, 2));
  }
}

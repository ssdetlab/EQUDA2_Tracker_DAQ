#include "TCycleAnalysis.h"
// #include "DBHelpers.h"
#include "TEnduranceCycle.h"

#include <string>


// TODO: write FIFO errors and exceptions to file (cycle file and recovery file)
// add cut on FIFO errors and exceptions
// increase classification number
// treat reading of recovery file correctly (with or without FIFO), according to classification
// number check adding of slices

TCycleAnalysis::TCycleAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan,
                               TScanConfig *aScanConfig, std::vector<THic *> hics,
                               std::mutex *aMutex, TCycleResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  if (aResult)
    m_result = aResult;
  else
    m_result = new TCycleResult();
}

void TCycleAnalysis::InitCounters()
{
  std::map<std::string, TScanResultHic *>::iterator it;
  for (it = m_result->GetHicResults()->begin(); it != m_result->GetHicResults()->end(); ++it) {
    std::cout << "found " << m_result->GetHicResults()->size() << "hic results, initialising "
              << it->first << std::endl;
    TCycleResultHic *result   = (TCycleResultHic *)it->second;
    result->m_weight          = 1;
    result->m_nTrips          = 0;
    result->m_minWorkingChips = 14;
    result->m_nChipFailures   = 0;
    result->m_nExceptions     = 0;
    result->m_nFifoTests      = 0;
    result->m_nFifoExceptions = 0;
    result->m_nFifoErrors     = 0;
    result->m_nFifoErrors0    = 0;
    result->m_nFifoErrors5    = 0;
    result->m_nFifoErrorsa    = 0;
    result->m_nFifoErrorsf    = 0;
    result->m_avDeltaT        = 0;
    result->m_maxDeltaT       = 0;
    result->m_avIdda          = 0;
    result->m_maxIdda         = 0;
    result->m_minIdda         = 999;
    result->m_avIddd          = 0;
    result->m_maxIddd         = 0;
    result->m_minIddd         = 999;
  }
}

// TODO write cycle variables to file
// Q: 1 file per HIC? -> yes, since attachment per activity, activity is per HIC
void TCycleAnalysis::Finalize()
{
  if (fScanAbort || fScanAbortAll) return;
  char                                            fName[200];
  std::vector<std::map<std::string, THicCounter>> counters =
      ((TEnduranceCycle *)m_scan)->GetCounters();
  ((TCycleResult *)m_result)->m_nCycles = counters.size();

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    TCycleResultHic *hicResult =
        (TCycleResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());

    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/CycleFile_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "CycleFile_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
              m_config->GetfNameSuffix());
    }
    hicResult->SetCycleFile(fName);
    FILE *fp = fopen(fName, "w");

    for (unsigned int icycle = 0; icycle < counters.size(); icycle++) {
      std::map<std::string, THicCounter> hicCounters = counters.at(icycle);
      THicCounter                        hicCounter  = hicCounters.at(m_hics.at(ihic)->GetDbId());

      fprintf(fp, "%d %d %d %d %d %.3f %.3f %.3f %.3f %.1f %.1f\n", icycle,
              hicCounter.m_trip ? 1 : 0, hicCounter.m_nWorkingChips, hicCounter.m_fifoErrors,
              hicCounter.m_fifoExceptions, hicCounter.m_iddaClocked, hicCounter.m_idddClocked,
              hicCounter.m_iddaConfigured, hicCounter.m_idddConfigured, hicCounter.m_tempStart,
              hicCounter.m_tempEnd);

      if (hicCounter.m_trip) hicResult->m_nTrips++;
      if (hicCounter.m_tempEnd - hicCounter.m_tempStart > hicResult->m_maxDeltaT)
        hicResult->m_maxDeltaT = hicCounter.m_tempEnd - hicCounter.m_tempStart;
      if (hicCounter.m_nWorkingChips < hicResult->m_minWorkingChips)
        hicResult->m_minWorkingChips = hicCounter.m_nWorkingChips;
      if (hicCounter.m_iddaClocked < hicResult->m_minIdda)
        hicResult->m_minIdda = hicCounter.m_iddaClocked;
      if (hicCounter.m_iddaClocked > hicResult->m_maxIdda)
        hicResult->m_maxIdda = hicCounter.m_iddaClocked;
      if (hicCounter.m_idddClocked < hicResult->m_minIddd)
        hicResult->m_minIddd = hicCounter.m_idddClocked;
      if (hicCounter.m_idddClocked > hicResult->m_maxIddd)
        hicResult->m_maxIddd = hicCounter.m_idddClocked;
      if (hicCounter.m_hicType == HIC_OB)
        hicResult->m_nChipFailures += (14 - hicCounter.m_nWorkingChips);
      else if (hicCounter.m_hicType == HIC_IB)
        hicResult->m_nChipFailures += (9 - hicCounter.m_nWorkingChips);

      hicResult->m_avDeltaT += hicCounter.m_tempEnd - hicCounter.m_tempStart;
      hicResult->m_avIdda += hicCounter.m_iddaClocked;
      hicResult->m_avIddd += hicCounter.m_idddClocked;
      hicResult->m_nExceptions += hicCounter.m_exceptions;
      hicResult->m_nFifoExceptions += hicCounter.m_fifoExceptions;
      hicResult->m_nFifoErrors += hicCounter.m_fifoErrors;
      hicResult->m_nFifoErrors0 += hicCounter.m_fifoErrors0;
      hicResult->m_nFifoErrors5 += hicCounter.m_fifoErrors5;
      hicResult->m_nFifoErrorsa += hicCounter.m_fifoErrorsa;
      hicResult->m_nFifoErrorsf += hicCounter.m_fifoErrorsf;
      hicResult->m_nFifoTests += hicCounter.m_fifoTests;
    }
    hicResult->SetValidity(true);
    fclose(fp);
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    TCycleResultHic *hicResult =
        (TCycleResultHic *)m_result->GetHicResults()->at(m_hics.at(ihic)->GetDbId());

    hicResult->m_avDeltaT /= ((TCycleResult *)m_result)->m_nCycles;
    hicResult->m_avIdda /= ((TCycleResult *)m_result)->m_nCycles;
    hicResult->m_avIddd /= ((TCycleResult *)m_result)->m_nCycles;
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      std::cout << "Classification not implemented for IB HIC" << std::endl;
    }
  }

  WriteResult();

  m_finished = true;
}


THicClassification TCycleAnalysis::GetClassificationOB(TCycleResultHic *result)
{
  THicClassification returnValue = CLASS_GOLD;

  DoCut(returnValue, CLASS_SILVER, result->m_nTrips, "ENDURANCEMAXTRIPSGREEN", result);
  DoCut(returnValue, CLASS_RED, result->m_nTrips, "ENDURANCEMAXTRIPSORANGE", result);
  DoCut(returnValue, CLASS_SILVER, result->m_minWorkingChips, "ENDURANCEMINCHIPSGREEN", result,
        true);
  DoCut(returnValue, CLASS_RED, result->m_nChipFailures, "ENDURANCEMAXFAILURESORANGE", result);
  std::cout << "Cycle Analysis - Classification: " << WriteHicClassification(returnValue)
            << std::endl;
  return returnValue;
}


// public helper method to reclassify HIC after combination of results.
THicClassification TCycleAnalysis::ReClassify(TCycleResultHic *result)
{
  THicClassification returnValue = GetClassificationOB(result);
  result->m_class                = returnValue;
  return returnValue;
}


void TCycleAnalysis::WriteResult()
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    if (!hicResult->IsValid()) continue;
    if (m_config->GetUseDataPath()) {
      sprintf(fName, "%s/CycleResult_%s.dat", hicResult->GetOutputPath().c_str(),
              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "CycleResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
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

void TCycleResult::WriteToFileGlobal(FILE *fp)
{
  fprintf(fp, "Number of cycles: %d\n\n", m_nCycles);
}

// void TCycleResultHic::WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
// {
  // std::string fileName, cycleName;
  // std::size_t slash;
  // if (m_class != CLASS_UNTESTED) { // adhoc modification to avoid writing pars of all slices
    // DbAddParameter(db, activity, string("Number of trips"), (float)m_nTrips, GetParameterFile());
    // DbAddParameter(db, activity, string("Min. number of working chips"), (float)m_minWorkingChips,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Number of chip failures"), (float)m_nChipFailures,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Av. delta T"), (float)m_avDeltaT, GetParameterFile());
    // DbAddParameter(db, activity, string("Max. delta T"), (float)m_maxDeltaT, GetParameterFile());
    // DbAddParameter(db, activity, string("Av. IDDA"), (float)m_avIdda, GetParameterFile());
    // DbAddParameter(db, activity, string("Min. IDDA"), (float)m_minIdda, GetParameterFile());
    // DbAddParameter(db, activity, string("Max. IDDA"), (float)m_maxIdda, GetParameterFile());
    // DbAddParameter(db, activity, string("Av. IDDD"), (float)m_avIddd, GetParameterFile());
    // DbAddParameter(db, activity, string("Min. IDDD"), (float)m_minIddd, GetParameterFile());
    // DbAddParameter(db, activity, string("Max. IDDD"), (float)m_maxIddd, GetParameterFile());
    // DbAddParameter(db, activity, string("FIFO errors (endurance)"), (float)m_nFifoErrors,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("FIFO exceptions (endurance)"), (float)m_nFifoExceptions,
                  //  GetParameterFile());
    // DbAddParameter(db, activity, string("Exceptions"), (float)m_nExceptions, GetParameterFile());
  // }
  // slash    = string(m_resultFile).find_last_of("/");
  // fileName = string(m_resultFile).substr(slash + 1); // strip path

  // slash     = string(m_cycleFile).find_last_of("/");
  // cycleName = string(m_cycleFile).substr(slash + 1); // strip path

  // DbAddAttachment(db, activity, attachResult, string(m_resultFile), fileName);
  // DbAddAttachment(db, activity, attachResult, string(m_cycleFile), cycleName);
// }

void TCycleResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf(fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Trips:                     %d\n", m_nTrips);
  fprintf(fp, "Min. number of chips:      %d\n", m_minWorkingChips);
  fprintf(fp, "Number of chip failures:   %d\n", m_nChipFailures);
  fprintf(fp, "Number of exceptions:      %d\n", m_nExceptions);
  fprintf(fp, "Number of FIFO errors:     %d\n", m_nFifoErrors);
  if (m_nFifoErrors > 0) {
    fprintf(fp, "  Pattern 0x0000:          %d\n", m_nFifoErrors0);
    fprintf(fp, "  Pattern 0x5555:          %d\n", m_nFifoErrors5);
    fprintf(fp, "  Pattern 0xaaaa:          %d\n", m_nFifoErrorsa);
    fprintf(fp, "  Pattern 0xffff:          %d\n", m_nFifoErrorsf);
  }
  fprintf(fp, "Number of FIFO exceptions: %d\n", m_nFifoExceptions);
  fprintf(fp, "Average delta T:           %.1f\n", m_avDeltaT);
  fprintf(fp, "Maximum delta T:           %.1f\n", m_maxDeltaT);
  fprintf(fp, "Average Idda:              %.3f\n", m_avIdda);
  fprintf(fp, "Maximum Idda:              %.3f\n", m_maxIdda);
  fprintf(fp, "Minimum Idda:              %.3f\n", m_minIdda);
  fprintf(fp, "Average Iddd:              %.3f\n", m_avIddd);
  fprintf(fp, "Maximum Iddd:              %.3f\n", m_maxIddd);
  fprintf(fp, "Minimum Iddd:              %.3f\n", m_minIddd);
}


// method to combine the results of different slices into one
// has to be called after WriteToFile(Finalize), before WriteToDB
// TODO: reclassify after
void TCycleResultHic::Add(TCycleResultHic &aResult)
{
  m_nTrips += aResult.m_nTrips;
  m_nChipFailures += aResult.m_nChipFailures;
  m_nFifoErrors += aResult.m_nFifoErrors;
  m_nFifoErrors0 += aResult.m_nFifoErrors0;
  m_nFifoErrors5 += aResult.m_nFifoErrors5;
  m_nFifoErrorsa += aResult.m_nFifoErrorsa;
  m_nFifoErrorsf += aResult.m_nFifoErrorsf;
  m_nFifoExceptions += aResult.m_nFifoExceptions;
  m_nExceptions += aResult.m_nExceptions;

  m_avDeltaT = (m_weight * m_avDeltaT + aResult.m_avDeltaT) / (m_weight + 1);
  m_avIdda   = (m_weight * m_avIdda + aResult.m_avIdda) / (m_weight + 1);
  m_avIddd   = (m_weight * m_avIddd + aResult.m_avIddd) / (m_weight + 1);

  if (aResult.m_maxDeltaT > m_maxDeltaT) m_maxDeltaT = aResult.m_maxDeltaT;
  if (aResult.m_maxIdda > m_maxIdda) m_maxIdda = aResult.m_maxIdda;
  if (aResult.m_maxIddd > m_maxIddd) m_maxIddd = aResult.m_maxIddd;

  if (aResult.m_minWorkingChips < m_minWorkingChips) m_minWorkingChips = aResult.m_minWorkingChips;
  if (aResult.m_minIdda < m_minIdda) m_minIdda = aResult.m_minIdda;
  if (aResult.m_minIddd < m_minIddd) m_minIddd = aResult.m_minIddd;

  m_weight++;
}


float TCycleResultChip::GetVariable(TResultVariable var)
{
  switch (var) {
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}

// TODO: Write to DB, classification

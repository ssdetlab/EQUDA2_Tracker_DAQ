#include "TScanFactory.h"
#include "TScan.h"
#include "TScanAnalysis.h"

#include "TAlpide.h"

#include "TApplyMask.h"
#include "TConfig.h"
#include "TCycleAnalysis.h"
#include "TDCTRLAnalysis.h"
#include "TDCTRLMeasurement.h"
#include "TDigitalAnalysis.h"
#include "TDigitalScan.h"
#include "TDigitalWFAnalysis.h"
#include "TEnduranceCycle.h"
#include "TEyeAnalysis.h"
#include "TEyeMeasurement.h"
#include "TFastPowerAnalysis.h"
#include "TFastPowerTest.h"
#include "TFifoAnalysis.h"
#include "TFifoTest.h"
#include "THIC.h"
#include "THisto.h"
#include "TLocalBusAnalysis.h"
#include "TLocalBusTest.h"
#include "TNoiseAnalysis.h"
#include "TNoiseOccupancy.h"
#include "TPowerAnalysis.h"
#include "TPowerTest.h"
#include "TReadoutAnalysis.h"
#include "TReadoutTest.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TThresholdAnalysis.h"

#ifdef HAS_ROOT
#include "TApplyTuning.h"
#include "TSCurveAnalysis.h"
#include "TSCurveScan.h"
#endif

TScanFactory::TScanObjects TScanFactory::CreateScanObjects(TScanType scanType, TScanConfig *config,
                                                           std::vector<TAlpide *>       chips,
                                                           std::vector<THic *>          hics,
                                                           std::vector<TReadoutBoard *> boards,
                                                           std::deque<TScanHisto> *     histoQue,
                                                           std::mutex *mutex, TScanResult *result)
{
  TScanObjects obj{nullptr, nullptr, nullptr, true};

  switch (scanType) {
  case STPower:
    obj.scan     = new TPowerTest(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TPowerResult();
    obj.analysis = new TPowerAnalysis(histoQue, (TPowerTest *)obj.scan, config, hics, mutex,
                                      (TPowerResult *)obj.result);
    break;

  case STFifo:
    obj.scan     = new TFifoTest(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TFifoResult();
    obj.analysis = new TFifoAnalysis(histoQue, (TFifoTest *)obj.scan, config, hics, mutex,
                                     (TFifoResult *)obj.result);
    break;

  case STDctrl:
    obj.scan     = new TDctrlMeasurement(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TDctrlResult();
    obj.analysis = new TDctrlAnalysis(histoQue, (TDctrlMeasurement *)obj.scan, config, hics, mutex,
                                      (TDctrlResult *)obj.result);
    break;

  case STLocalBus:
    obj.scan     = new TLocalBusTest(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TLocalBusResult();
    obj.analysis = new TLocalBusAnalysis(histoQue, (TLocalBusTest *)obj.scan, config, hics, mutex,
                                         (TLocalBusResult *)obj.result);
    break;

  case STDigital:
    obj.scan     = new TDigitalScan(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TDigitalResult();
    obj.analysis = new TDigitalAnalysis(histoQue, (TDigitalScan *)obj.scan, config, hics, mutex,
                                        (TDigitalResult *)obj.result);
    break;

  case STDigitalWF:
    obj.scan     = new TDigitalWhiteFrame(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TDigitalWFResult();
    obj.analysis = new TDigitalWFAnalysis(histoQue, (TDigitalWhiteFrame *)obj.scan, config, hics,
                                          mutex, (TDigitalWFResult *)obj.result);
    break;

  case STNoise:
    obj.scan     = new TNoiseOccupancy(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TNoiseResult();
    obj.analysis = new TNoiseAnalysis(histoQue, (TNoiseOccupancy *)obj.scan, config, hics, mutex,
                                      (TNoiseResult *)obj.result);
    break;

  case STReadout:
    obj.scan     = new TReadoutTest(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TReadoutResult();
    obj.analysis = new TReadoutAnalysis(histoQue, (TReadoutTest *)obj.scan, config, hics, mutex,
                                        (TReadoutResult *)obj.result);
    break;

  case STEndurance:
    obj.scan     = new TEnduranceCycle(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TCycleResult();
    obj.analysis = new TCycleAnalysis(histoQue, (TEnduranceCycle *)obj.scan, config, hics, mutex,
                                      (TCycleResult *)obj.result);
    break;

  case STFastPowerTest:
    obj.scan     = new TFastPowerTest(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TFastPowerResult();
    obj.analysis = new TFastPowerAnalysis(histoQue, (TFifoTest *)obj.scan, config, hics, mutex,
                                          (TFastPowerResult *)obj.result);
    break;

#ifdef HAS_ROOT

  case STThreshold:
    obj.scan     = new TThresholdScan(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TSCurveResult();
    obj.analysis = new TSCurveAnalysis(histoQue, (TThresholdScan *)obj.scan, config, hics, mutex,
                                       (TSCurveResult *)obj.result);
    break;

  case STEyeScan:
    obj.scan     = new TEyeMeasurement(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TEyeResult();
    obj.analysis = new TEyeAnalysis(histoQue, (TEyeMeasurement *)obj.scan, config, hics, mutex,
                                    (TEyeResult *)obj.result);
    break;

  case STVCASN:
    obj.scan     = new TtuneVCASNScan(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TSCurveResult();
    obj.analysis = new TSCurveAnalysis(histoQue, (TtuneVCASNScan *)obj.scan, config, hics, mutex,
                                       (TSCurveResult *)obj.result, 1);
    break;

  case STITHR:
    obj.scan     = new TtuneITHRScan(config, chips, hics, boards, histoQue, mutex);
    obj.result   = new TSCurveResult();
    obj.analysis = new TSCurveAnalysis(histoQue, (TtuneITHRScan *)obj.scan, config, hics, mutex,
                                       (TSCurveResult *)obj.result, -1);
    break;

  // apply tuning masks: scan = 0, analysis gets previous result as input
  // result value has to stay unchanged here; however AddScan will push back 0 into result vector
  case STApplyITHR:
    obj.analysis  = new TApplyITHRTuning(histoQue, 0, config, hics, mutex, (TSCurveResult *)result);
    obj.hasButton = false;
    break;

  case STApplyVCASN:
    obj.analysis = new TApplyVCASNTuning(histoQue, 0, config, hics, mutex, (TSCurveResult *)result);
    obj.hasButton = false;
    break;

  case STApplyMask:
    obj.analysis  = new TApplyMask(histoQue, 0, config, hics, mutex, (TNoiseResult *)result);
    obj.hasButton = false;
    break;

  case STClearMask:
    obj.analysis  = new TApplyMask(histoQue, 0, config, hics, mutex, 0);
    obj.hasButton = false;
    break;

#endif

  default:
    obj.hasButton = false;
    std::cout << "Warning: unknown scantype " << (int)scanType << ", ignoring" << std::endl;
  }

  return obj;
}

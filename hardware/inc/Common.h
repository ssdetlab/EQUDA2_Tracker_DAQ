// NAMESPACE CONTAINING TYPES AND FUNTIONS
// FOR COMMON USE IN ANALYSIS AND SCAN CLASSES.

#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <string>
#include <vector>

#include "AlpideDecoder.h"
//#include "THIC.h"

class TScanHisto;
class THic;

namespace common {

  const int nRows = 512;
  const int nCols = 1024;

  typedef struct {
    int boardIndex;
    int dataReceiver;
    int chipId;
  } TChipIndex;

  typedef struct {
    double threshold;
    double noise;
    double redChi2;
  } TErrFuncFitResult;

  typedef struct {
    unsigned int boardIndex;
    unsigned int dataReceiver;
    unsigned int chipId;

    int vPulseL;
    int vPulseH;
    int vPulseStep;
    int nMask;

    int counterPixelsNoHits;
    int counterPixelsStuck;
    int counterPixelsNoThreshold;

    double thresholdMean;
    double thresholdStdDev;
    double noiseMean;
    double noiseStdDev;

    FILE *filePixelNoHits;
    FILE *filePixelStuck;
    FILE *filePixelNoThreshold; // To do, based on chi2 cut?
    FILE *filePixelFitResult;
    FILE *fileRawData;

  } TThresholdResult;

  typedef struct {
    double sum;
    double sum2;
    int    entries;

    double mean;
    double stdDev;

  } TStatVar;

  extern std::string             GetFileName(TChipIndex aChipIndex, std::string suffix);
  extern int                     GetFileName();
  extern int                     GetChipIntIndex(TChipIndex aChipIndex);
  extern TChipIndex              GetChipIndex(int aIntIndex);
  extern std::vector<TChipIndex> GetChipList(TScanHisto *aScanHisto);

  bool HitBelongsToChip(TChipIndex aChipIndex, TPixHit aHit);
  bool HitBelongsToHic(THic *aHic, TPixHit aHit);
  int  FindIndexForHit(std::vector<TChipIndex> aChipList, TPixHit aHit);
  bool PixelAlreadyHit(std::vector<TPixHit> *pixels, TPixHit aHit);
  bool DColAlreadyHit(std::vector<TPixHit> *pixels, TPixHit aHit);
} // namespace common

#endif

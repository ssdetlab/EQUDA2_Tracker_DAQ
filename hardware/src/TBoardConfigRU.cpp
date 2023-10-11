#include "TBoardConfigRU.h"

#include "TReadoutBoardRU.h"

TBoardConfigRU::TBoardConfigRU(const char *fName, int boardIndex) : TBoardConfig(fName, boardIndex)
{
  this->fBoardType = TBoardType::boardRU;
  InitParamMap();
}

uint8_t TBoardConfigRU::getTransceiverChip(const uint8_t DP, const uint8_t index) const
{
  std::vector<uint8_t> mapping_dp0 = {0, 2, 4, 6, 8, 10, 12, 255};
  std::vector<uint8_t> mapping_dp1 = {1, 3, 5, 7, 9, 11, 13, 255};

  if (index > 7) return 255;
  if (DP == TReadoutBoardRU::EP_DATA0_IN)
    return mapping_dp0[index];
  else if (DP == TReadoutBoardRU::EP_DATA1_IN)
    return mapping_dp1[index];
  return 255;
}

std::vector<TBoardConfigRU::TransceiverMapping> TBoardConfigRU::getTransceiverMappings()
{
  std::vector<TBoardConfigRU::TransceiverMapping> mapping;
  for (int i = 0; i < 9; ++i) {
    mapping.emplace_back(i, i);
  }
  return mapping;
}

uint8_t TBoardConfigRU::getConnector() const { return 0; }

TBoardConfigRU::ReadoutSpeed TBoardConfigRU::getReadoutSpeed() const
{
  return TBoardConfigRU::ReadoutSpeed::RO_1200;
}
bool TBoardConfigRU::getInvertPolarity() const { return false; }

bool TBoardConfigRU::enableLogging() const { return true; }

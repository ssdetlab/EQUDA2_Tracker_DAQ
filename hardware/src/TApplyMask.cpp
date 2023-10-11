#include "TApplyMask.h"

TApplyMask::TApplyMask(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                       std::vector<THic *> hics, std::mutex *aMutex, TNoiseResult *aResult)
    : TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex)
{
  m_result = aResult;
}

void TApplyMask::Run()
{
  if (m_result) { // result with mask present -> mask noisy pixels
    m_config->SetIsMasked(true);

    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      TNoiseResultHic *hicResult =
          (TNoiseResultHic *)m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
      std::map<int, TScanResultChip *>::iterator it;

      for (it = hicResult->m_chipResults.begin(); it != hicResult->m_chipResults.end(); ++it) {
        TAlpide *         chip       = m_hics.at(ihic)->GetChipById(it->first);
        TNoiseResultChip *chipResult = (TNoiseResultChip *)it->second;

        chip->GetConfig()->SetNoisyPixels(chipResult->GetNoisyPixels());
      }
    }
  }
  else { // no result -> clear masks
    m_config->SetIsMasked(false);
    for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
      THic *                 hic   = m_hics.at(ihic);
      std::vector<TAlpide *> chips = hic->GetChips();
      for (unsigned int ichip = 0; ichip < chips.size(); ichip++) {
        TAlpide *chip = chips.at(ichip);
        chip->GetConfig()->ClearNoisyPixels();
      }
    }
  }
}

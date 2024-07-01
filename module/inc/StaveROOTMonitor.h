#pragma once
#include "eudaq/ROOTMonitor.hh"
#include "TCanvas.h"
#include "TH2I.h"

#include "AlpideDecoder.h"
#include "BoardDecoder.h"


#ifndef StaveROOTMonitor_h
#define StaveROOTMonitor_h

class StaveROOTMonitor : public eudaq::ROOTMonitor{
    public:
        StaveROOTMonitor(const std::string& name, const std::string& runcontrol)
            : eudaq::ROOTMonitor(name, "Stave ROOT monitor", runcontrol){}

        void AtConfiguration() override;
        void AtEventReception(eudaq::EventSP ev) override;

        static const uint32_t m_id_factory = eudaq::cstr2hash("StaveROOTMonitor");

        // MOSAIC-level error counters
        int end_of_run;
        int overflow;
        int timeout;
        int header_error;
        int decoder_10b8b_error;
        int event_oversize_error;

        // ALPIDE-level error counters
        int n_corrupted_chip;
        int prio_errs;

    private:
        TCanvas* canvas;
        std::map<int,std::vector<TH2I*>> pix_grid;
        int nof_staves;
        std::map<int,int> stave_to_act_chips;
};

namespace{
    auto mon_rootmon = eudaq::Factory<eudaq::Monitor>::
        Register<StaveROOTMonitor, const std::string&, const std::string&>
            (StaveROOTMonitor::m_id_factory);
}

#endif
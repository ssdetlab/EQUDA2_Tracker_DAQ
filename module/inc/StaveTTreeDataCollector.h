#ifndef StaveTTreeDataCollector_h
#define StaveTTreeDataCollector_h

#include "eudaq/DataCollector.hh"

#include "DetectorEvent.h"
#include "circ_buffer.h"

#include "TTree.h"
#include "TFile.h"

#include "AlpideDecoder.h"
#include "BoardDecoder.h"

#include <unordered_map>
#include <omp.h>

class StaveTTreeDataCollector : public eudaq::DataCollector {
    public:
        using eudaq::DataCollector::DataCollector;
        void DoStopRun()                                           override;
        void DoConfigure()                                         override;
        void DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) override;
        void DoTerminate()                                         override;
        void DoReset()                                             override;

        static const uint32_t m_id_factory = eudaq::cstr2hash("StaveTTreeDataCollector");
    private:
        // constants for data organization 
        int n_staves;
        int min_ev_to_dump;
        int min_staves;

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

        // to dump data to
        std::shared_ptr<TFile> tree_file;
        std::shared_ptr<TTree> data_tree;

        // custom class to store event data in the tree 
        detector_event det_ev;

        // buffers to temporary store data and sync chip events 
        // and to keep track of the insertion order
        std::unordered_map<
            std::uint32_t,
            std::unordered_map<
                std::uint8_t,
                std::vector<chip_event>>> temp_chip_ev_buffer;
        std::shared_ptr<circ_buffer<std::uint32_t>> ev_ins_ord;  

        // online monitor protection
        bool online;
        std::mutex online_mtx;

        bool configured = false;
};

namespace {
    auto dummy0 = eudaq::Factory<eudaq::DataCollector>::
        Register<StaveTTreeDataCollector, const std::string&, const std::string&>
        (StaveTTreeDataCollector::m_id_factory);
}

#endif

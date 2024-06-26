#ifndef StaveTTreeDataCollectorTLUSync_h
#define StaveTTreeDataCollectorTLUSync_h

#include "eudaq/DataCollector.hh"

#include "DetectorEvent.h"
#include "circ_buffer.h"

#include "TTree.h"
#include "TFile.h"

#include "AlpideDecoder.h"
#include "BoardDecoder.h"

#include <set>
#include <omp.h>

class StaveTTreeDataCollectorTLUSync : public eudaq::DataCollector {
    public:
        using eudaq::DataCollector::DataCollector;
        void DoStopRun()                                           override;
        void DoConfigure()                                         override;
        void DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) override;
        void DoTerminate()                                         override;
        void DoReset()                                             override;

        static const uint32_t m_id_factory = eudaq::cstr2hash("StaveTTreeDataCollectorTLUSync");
    private:
        // constants for data organization 
        int n_staves;
        int min_ev_to_dump;
        int min_staves;

        // error packet counters
        int header_error;
        int errors8b10b;
        int oversize_error;

        // to dump data to
        std::shared_ptr<TFile> tree_file;
        std::shared_ptr<TTree> data_tree;

        // custom class to store event data in the tree 
        detector_event_tlu det_ev;

        // buffers to temporary store data and sync chip events 
        // and to keep track of the insertion order
        std::map<std::uint16_t,std::map<std::uint8_t,std::vector<chip_event>>> temp_chip_ev_buffer;
        std::map<std::uint16_t,tlu_event> temp_tlu_ev_buffer;
        std::shared_ptr<circ_buffer<std::uint16_t>> ev_ins_ord;  

        // online monitor protection
        bool online;
        std::mutex online_mtx;

        bool configured = false;
};

namespace {
    auto dummy0 = eudaq::Factory<eudaq::DataCollector>::
        Register<StaveTTreeDataCollectorTLUSync, const std::string&, const std::string&>
        (StaveTTreeDataCollectorTLUSync::m_id_factory);
}

#endif

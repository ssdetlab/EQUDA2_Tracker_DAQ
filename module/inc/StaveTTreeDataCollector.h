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
        void DoStartRun()                                          override;
        void DoConfigure()                                         override;
        void DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) override;
        void DoTerminate()                                         override;
        void DoReset()                                             override;

        static const uint32_t m_id_factory = eudaq::cstr2hash("StaveTTreeDataCollector");
    private:
        void DumpFile();
        void PrepareFileTree();

        // constants for data organization 
        int n_staves;
        int min_ev_to_dump;
        int min_staves;
        
        int data_limit;
        std::string date_time_format;
        std::string file_tree_path;
        std::string tree_name;
        int buf_size;
        int split_lvl;

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

        bool configured = false;
};

namespace {
    auto dummy0 = eudaq::Factory<eudaq::DataCollector>::
        Register<StaveTTreeDataCollector, const std::string&, const std::string&>
            (StaveTTreeDataCollector::m_id_factory);
}

#endif

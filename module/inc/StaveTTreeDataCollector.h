#pragma once
#include "eudaq/DataCollector.hh"

#include "DetectorEvent.h"
#include "circ_buffer.h"

#include "TTree.h"
#include "TFile.h"

#include "AlpideDecoder.h"
#include "BoardDecoder.h"

#include <set>
#include <omp.h>

#ifndef StaveTTreeDataCollector_h
#define StaveTTreeDataCollector_h

class StaveTTreeDataCollector : public eudaq::DataCollector{
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

        // error packet counters
        int header_error;
        int errors8b10b;
        int oversize_error;

        // to dump data to
        std::shared_ptr<TFile> tree_file;
        std::shared_ptr<TTree> data_tree;

        // custom class to store event data in the tree 
        detector_event det_ev;

        // buffers to temporary store data and sync chip events 
        // and to keep track of the insertion order
        std::map<unsigned int,std::vector<stave_event>> temp_ev_buffer;
        std::shared_ptr<circ_buffer<unsigned int>> ev_ins_ord;  

        // online monitor protection
        bool online;
        std::mutex online_mtx;

        bool configured = false;
};

namespace{
    auto dummy0 = eudaq::Factory<eudaq::DataCollector>::
        Register<StaveTTreeDataCollector, const std::string&, const std::string&>
        (StaveTTreeDataCollector::m_id_factory);
}

#endif
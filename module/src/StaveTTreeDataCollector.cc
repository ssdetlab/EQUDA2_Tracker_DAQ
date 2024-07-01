#include "StaveTTreeDataCollector.h"
#include "TLUPayloadDecoder.h"

#include <fstream>
#include <chrono>

void StaveTTreeDataCollector::DoConfigure() {
    auto conf = GetConfiguration();
    
    // set ROOT tree parameters
    file_tree_path = conf->Get("TTREE_DATA_PATH", "tree.root");
    tree_name = conf->Get("TTREE_NAME", "MyTree");
    buf_size  = conf->Get("BUF_SIZE",  32000);
    split_lvl = conf->Get("SPLIT_LVL", 0);

    // for ROOT file and tree initializaiton
    data_limit = conf->Get("FILE_SWITCH_LIMIT", 1000);
    date_time_format = conf->Get("DATE_TIME_FORMAT", "_%m_%d_%Y_%H:%M:%S");

    std::cout << "\n\n\n" << std::endl;
    std::cout << data_limit << std::endl;
    std::cout << "\n\n\n" << std::endl;

    // set number of active boards, cut on minimal 
    // amount of events tp dump into the tree and 
    // minimal number of planes involved into an 
    // event to dump it
    n_staves       = conf->Get("NOF_STAVES",     1);
    min_ev_to_dump = conf->Get("MIN_EV_TO_DUMP", 10);
    
    // initialize cyclic buffer to keep track
    // of the insertion order
    ev_ins_ord = std::make_shared<circ_buffer<std::uint32_t>>(n_staves*min_ev_to_dump*2);
    
    configured = true;
}

void StaveTTreeDataCollector::DoStartRun() {
    // prepare the ROOT file and the tree
    PrepareFileTree();
}

void StaveTTreeDataCollector::PrepareFileTree() {
    // check the cirrent time and 
    // format it for the file naming
    auto now = std::chrono::system_clock::now();

    time_t time
        = std::chrono::system_clock::to_time_t(now);
    tm* timeinfo = localtime(&time);
    char buffer[70];
    strftime(buffer, sizeof(buffer), date_time_format.c_str(),
        timeinfo);

    std::string formatted_time = buffer;

    std::string file_path = file_tree_path; 
    file_path.insert(file_path.find_last_of('.'), formatted_time);

    // Set up the file and the tree
    tree_file = std::make_shared<TFile>(file_path.c_str(), "recreate");
    data_tree = std::make_shared<TTree>(tree_name.c_str(), tree_name.c_str());

    data_tree->Branch("event", &det_ev, buf_size, split_lvl);
}

void StaveTTreeDataCollector::DumpFile() {
    std::vector<stave_event> st_ev_buff;
    for (auto& [id,stevs] : temp_chip_ev_buffer) {
        for (auto& stev : stevs) {
            stave_event st_ev;
            st_ev.trg_n = id;
            st_ev.stave_id = stev.first;
            st_ev.ch_ev_buffer = stev.second;
            st_ev_buff.push_back(st_ev); 
        }
        det_ev.st_ev_buffer = st_ev_buff;
    
        data_tree->Fill();
    }

    data_tree->Write();
    tree_file->Close();
}

void StaveTTreeDataCollector::DoReceive(eudaq::ConnectionSPC idx, eudaq::EventSP ev) {
    // size holders and counters
    int n_bytes_header, n_bytes_trailer;
    int prio_err;

    // set up stave event
    std::uint8_t st_id = static_cast<std::uint8_t>(eudaq::from_string<std::uint16_t>(ev->GetTag("Stave ID")));

    // iterate over the chips data
    std::vector<std::uint8_t> block = ev->GetBlock(st_id);

    // distribute frame decoding between several threads
    #pragma omp parallel for num_threads(block.size()/MAX_EVENT_SIZE)
    for (int i = 0; i < block.size()/MAX_EVENT_SIZE; i++) {
        chip_event ch_ev;

        // each chip frame is in a separate 
        // MAX_EVENT_SIZE block of data
        std::uint8_t* buffer = &(block.at(i*MAX_EVENT_SIZE));

        // extract frame size for the MOSAIC decoder
        int n_bytes_data; 
        #pragma omp critical
        {
            if (ev->HasTag("Event size " + std::to_string(i))) {
                n_bytes_data = 
                    eudaq::from_string<std::int32_t>(ev->GetTag("Event size " + std::to_string(i)));
            }
        }
        // decode MOSAIC event
        TBoardHeader board_info;
        BoardDecoder::DecodeEventMOSAIC(
            buffer, n_bytes_data, n_bytes_header, 
            n_bytes_trailer, board_info);
        
        // fill MOSAIC flags
        ch_ev.channel              = board_info.channel; 
        ch_ev.overflow             = board_info.overflow;
        ch_ev.end_of_run           = board_info.endOfRun;
        ch_ev.timeout              = board_info.timeout;
        ch_ev.header_error         = board_info.headerError;
        ch_ev.decoder_10b8b_error  = board_info.decoder10b8bError;
        ch_ev.event_oversize_error = board_info.eventOverSizeError;

        // check for bad frame 
        bool bad_mosaic = 
            board_info.overflow ||
            board_info.endOfRun ||
            board_info.timeout ||
            board_info.headerError ||
            board_info.decoder10b8bError ||
            board_info.eventOverSizeError;

        // decode Chip event
        int n_bytes_chipevent = 
            n_bytes_data - 
            n_bytes_header - 
            n_bytes_trailer;

        std::vector<TPixHit> hits;
        int alpide_flags;
        unsigned int bc_counter;
        bool not_corrrupted = 
            AlpideDecoder::DecodeEvent(
                buffer + n_bytes_header, n_bytes_chipevent, &hits, st_id,
                board_info.channel, prio_err, alpide_flags, 40000000, 
                nullptr, nullptr, &bc_counter);

        // convert hits to a more convenient
        // data structure
        if (hits.size() == 0) {
            continue;
        }
        ch_ev.chip_id = hits[0].chipId;
        ch_ev.channel = hits[0].channel;

        // decode region->double_column->address
        // data structure into a hit-map
        if (!bad_mosaic && not_corrrupted & (alpide_flags&0x8) != 1) {
            for (auto hit : hits) {
                std::uint16_t rem = hit.address % 4;
                if (rem == 0 || rem == 3) {
                    rem = 0;
                }
                else {
                    rem = 1;
                }
                std::uint16_t pix_x = hit.region*32 + hit.dcol*2 + rem;
                std::uint16_t pix_y = hit.address/2;
                ch_ev.hits.push_back(std::make_tuple(pix_x,pix_y));
            }
        }
        // fill ALPIDE flags
        ch_ev.is_flushed_incomplete = (alpide_flags&0x4);
        ch_ev.is_strobe_extended    = (alpide_flags&0x2);
        ch_ev.is_busy_transition    = (alpide_flags&0x1);

        // only 0-1 thread at a time can access this 
        #pragma omp critical
        {
            // store stave event in the temporary buffer
            // to syncronize several chips and dump it into tree later 
            temp_chip_ev_buffer[bc_counter][st_id].push_back(ch_ev);
            ev->SetTriggerN(bc_counter);
        }
    }
    
    // currently the best option,
    // given that no trigger ID is 
    // supplied yet
    ev_ins_ord->write(ev->GetTriggerN());

    // if event queue is read out and we've reached minimal event
    // number threshold start dumping events
    if (ev_ins_ord->get_distance() > min_ev_to_dump) {
        // get oldest events trigger ids to dump them into tree
        std::uint32_t coff = 
            static_cast<std::uint32_t>(min_ev_to_dump/2.);
        std::vector<std::uint32_t>::iterator
            oldest_ev = ev_ins_ord->ptr_position(); 

        // dump oldest events
        for (auto it = oldest_ev; it != oldest_ev + coff; it++) {
            std::uint32_t id = *it;
            std::vector<stave_event> st_ev_buff;
            if (temp_chip_ev_buffer.find(id) == temp_chip_ev_buffer.end()) {
                continue;
            }
            for (auto& chev : temp_chip_ev_buffer[id]) {
                stave_event st_ev;
                st_ev.trg_n = id;
                st_ev.stave_id = chev.first;
                st_ev.ch_ev_buffer = chev.second;
                st_ev_buff.push_back(st_ev);
            }
            det_ev.st_ev_buffer = st_ev_buff;
            
            data_tree->Fill();
        
            // clean buffer from dumped events
            temp_chip_ev_buffer.erase(id);
        }
    
        // clean insertion order tracker from 
        // dumped events trigger ids
        ev_ins_ord->inc_read(coff);
    }
    
    // increase buffer size if we've reached "dangerous" point
    // (useful for high event rates)
    if (ev_ins_ord->get_distance() == 
        static_cast<std::uint32_t>(ev_ins_ord->size()/2.)) { 
            ev_ins_ord->resize(ev_ins_ord->size());
    }

    // send event to the monitor 
    SendEvent(std::move(ev));

    // Dump the data, save the file,
    // and switch to the next one 
    // if the size limit is reached
    std::cout << tree_file->GetBytesWritten()/1000000. << " > " << data_limit << std::endl;
    if (tree_file->GetBytesWritten()/1000000. > data_limit) {
        DumpFile();
        PrepareFileTree();
    }
}

void StaveTTreeDataCollector::DoStopRun() {
    // Dump the rest of the data
    DumpFile();
}

void StaveTTreeDataCollector::DoReset() {
    configured = false;
}

void StaveTTreeDataCollector::DoTerminate() {
    configured = false;
}
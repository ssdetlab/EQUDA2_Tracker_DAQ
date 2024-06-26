#include "StaveTTreeDataCollectorTLUSync.h"
#include "TLUPayloadDecoder.h"
#include <fstream>

// TODO:: take into account EndOfEvent
//        flag when syncronizing stave
//        data

void StaveTTreeDataCollectorTLUSync::DoConfigure() {
    auto conf = GetConfiguration();
    
    // set ROOT tree parameters
    std::string file_tree_path = conf->Get("TTREE_DATA_PATH", "tree.root");
    std::string tree_name = conf->Get("TTREE_NAME", "MyTree");
    int buf_size  = conf->Get("BUF_SIZE",  32000);
    int split_lvl = conf->Get("SPLIT_LVL", 0);
    
    online = conf->Get("ONLINE_MONITOR", false);

    // initialize ROOT file and tree
    tree_file = std::make_shared<TFile>(file_tree_path.c_str(), "recreate");
    data_tree = std::make_shared<TTree>(tree_name.c_str(), tree_name.c_str());
    
    data_tree->Branch("event", &det_ev, buf_size, split_lvl);
    
    // set number of active boards, cut on minimal 
    // amount of events tp dump into the tree and 
    // minimal number of planes involved into an 
    // event to dump it
    n_staves       = conf->Get("NOF_STAVES",     1);
    min_ev_to_dump = conf->Get("MIN_EV_TO_DUMP", 10);
    min_staves     = conf->Get("MIN_STAVES",     1);
    
    // initialize cyclic buffer to keep track
    // of the insertion order
    ev_ins_ord = std::make_shared<circ_buffer<std::uint16_t>>(n_staves*min_ev_to_dump*2);
    
    // initiazlize error packet counters 
    header_error   = 0;
    errors8b10b    = 0;
    oversize_error = 0;
    
    configured = true;
}

void StaveTTreeDataCollectorTLUSync::DoReceive(eudaq::ConnectionSPC idx, eudaq::EventSP ev) {
    if (ev->GetDescription() == "TluRawDataEvent") {
        tlu_event event;
    
        event.trg_n       = ev->GetTriggerN();
        event.event_begin = ev->GetTimestampBegin();
        event.event_end   = ev->GetTimestampEnd();
    
        event.trg_sign  = ev->GetTag("TRIGGER");
        event.fine_ts_0 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS0")));
        event.fine_ts_1 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS1")));
        event.fine_ts_2 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS2")));
        event.fine_ts_3 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS3")));
        event.fine_ts_4 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS4")));
        event.fine_ts_5 = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("FINE_TS5")));
        event.type      = 
            static_cast<std::uint8_t>(
                eudaq::from_string<std::uint16_t>(ev->GetTag("TYPE")));
    
        event.particles = eudaq::from_string<std::uint16_t>(ev->GetTag("PARTICLES"));
        event.scaler_0  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER0"));
        event.scaler_1  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER1"));
        event.scaler_2  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER2"));
        event.scaler_3  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER3"));
        event.scaler_4  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER4"));
        event.scaler_5  = eudaq::from_string<std::uint16_t>(ev->GetTag("SCALER5"));
    
        temp_tlu_ev_buffer[ev->GetTriggerN()] = event;
        ev_ins_ord->write(ev->GetTriggerN());
    }
    if (ev->GetDescription() == "StaveRaw") {
        // size holders and counters
        int n_bytes_header, n_bytes_trailer;
        int n_corrupted_chip = 0, prio_err = 0;
    
        // set up stave event
        std::uint8_t st_id = static_cast<std::uint8_t>(eudaq::from_string<std::uint16_t>(ev->GetTag("Stave ID")));
    
        // iterate over the chips data
        std::vector<std::uint8_t> block = ev->GetBlock(st_id);
        // distribute frame decoding between several threads
        #pragma omp parallel for num_threads(block.size()/MAX_EVENT_SIZE)
        for (int i = 0; i < block.size()/MAX_EVENT_SIZE; i ++) {
            chip_event ch_ev;

            // each chip frame is in a separate 
            // MAX_EVENT_SIZE block of data
            std::uint8_t buffer[MAX_EVENT_SIZE];
            for (int j = 0; j < MAX_EVENT_SIZE; j++) {
                buffer[j] = block[i*MAX_EVENT_SIZE + j];
            }
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
            // check empty event
            if (n_bytes_trailer == 0) {
                continue;
            }
            // check damaged frame 
            if (board_info.headerError) {
                header_error++;
                continue;
            } 
            if (board_info.decoder10b8bError) {
                errors8b10b++;
                continue;
            } 
            if (board_info.eventOverSizeError) {
                oversize_error++;
                continue;
            }
            // fill MOSAIC flags
            ch_ev.channel    = board_info.channel; 
            ch_ev.overflow   = board_info.overflow;
            ch_ev.end_of_run = board_info.endOfRun;
            // decode Chip event
            std::uint16_t tlu_trig_n;
            bool tlu_not_corrupted = 
                TLUPayloadDecoder::DecodeEvent(
                    buffer + 
                    n_bytes_data - 
                    TLUPayloadDecoder::DSIZE_TLU_PAYLOAD - 
                    n_bytes_trailer, tlu_trig_n);

            if (!tlu_not_corrupted) {
                n_corrupted_chip++;
                continue;
            }

            int n_bytes_chipevent = 
                n_bytes_data - 
                n_bytes_header - 
                n_bytes_trailer - 
                TLUPayloadDecoder::DSIZE_TLU_PAYLOAD;

            std::vector<TPixHit> hits;
            int alpide_flags;
            bool not_corrrupted = 
                AlpideDecoder::DecodeEvent(
                    buffer + n_bytes_header, n_bytes_chipevent, &hits, st_id,
                    board_info.channel, prio_err, alpide_flags, 40000000);
            // check event corruption (second condition -- BUSY VIOLATION)
            if (!not_corrrupted || (alpide_flags&0x8) == 1) {
                n_corrupted_chip++;
                continue;
            }
            // convert hits to a more convenient
            // data structure
            if (hits.size() == 0) {
                continue;
            }
            ch_ev.chip_id = hits[0].chipId;
            ch_ev.channel = hits[0].channel;
            // decode region->double_column->address
            // data structure into a hit-map
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
            // std::cout << "ch_ev.hits.size() = " << ch_ev.hits.size() << std::endl;
            // fill ALPIDE flags
            ch_ev.is_flushed_incomplete = (alpide_flags&0x4);
            ch_ev.is_strobe_extended    = (alpide_flags&0x2);
            ch_ev.is_busy_transition    = (alpide_flags&0x1);

            // only 0-1 thread at a time can access this 
            #pragma omp critical
            {
                // store stave event in the temporary buffer
                // to syncronize several chips and dump it into tree later 
                temp_chip_ev_buffer[tlu_trig_n][st_id].push_back(ch_ev);
                ev->SetTriggerN(tlu_trig_n);
            }
        }
    }

    // if event queue is read out and we've reached minimal event
    // number threshold start dumping events
    if (ev_ins_ord->get_distance() > min_ev_to_dump) {
        // get oldest events trigger ids to dump them into tree
        std::uint16_t coff = 
            static_cast<std::uint16_t>(min_ev_to_dump/2.);
        std::vector<std::uint16_t> 
            oldest_ev = ev_ins_ord->const_slice(coff); 

        // dump oldest events
        for (auto i : oldest_ev) {
            // check if we have corresponding tlu event
            if (temp_chip_ev_buffer.count(i) != 0) {
                std::vector<stave_event> st_ev_buff;
                for (auto& chev : temp_chip_ev_buffer[i]) {
                    stave_event st_ev;
                    st_ev.trg_n = i;
                    st_ev.stave_id = chev.first;
                    st_ev.ch_ev_buffer = chev.second;
                    st_ev_buff.push_back(st_ev);
                }

                det_ev.st_ev_buffer = st_ev_buff;
            }
            else {
                det_ev.st_ev_buffer.clear();
            }
            det_ev.tl_ev = temp_tlu_ev_buffer[i];
            
            data_tree->Fill();
        
            // the safest way to update 
            // the file in the online mode
            if (online) { 
                std::unique_lock<std::mutex> lock(online_mtx);
                tree_file->Write(0, TObject::kWriteDelete);
            }
        
            // clean buffer from dumped events
            temp_chip_ev_buffer.erase(i);
            temp_tlu_ev_buffer.erase(i);
        }
    
        // clean insertion order tracker from 
        // dumped events trigger ids
        ev_ins_ord->inc_read(coff);
    }
    // increase buffer size if we've reached "dangerous" point
    // (useful for high event rates)
    if (ev_ins_ord->get_distance() == 
        static_cast<std::uint16_t>(ev_ins_ord->size()/2.)) { 
            ev_ins_ord->resize(ev_ins_ord->size());
    }
    WriteEvent(std::move(ev));
}

void StaveTTreeDataCollectorTLUSync::DoStopRun() {
    // dump remaining events
    for (auto& tlu_ev : temp_tlu_ev_buffer) {
        // check if we have corresponding chip event
        if (temp_chip_ev_buffer.count(tlu_ev.first) != 0) {
            std::vector<stave_event> st_ev_buff;
            for (auto& chev : temp_chip_ev_buffer[tlu_ev.first]) {
                stave_event st_ev;
                st_ev.trg_n = tlu_ev.first;
                st_ev.stave_id = chev.first;
                st_ev.ch_ev_buffer = chev.second;
                st_ev_buff.push_back(st_ev); 
            }

            det_ev.st_ev_buffer = st_ev_buff;
        }
        else {
            det_ev.st_ev_buffer.clear();
        }
        det_ev.tl_ev = tlu_ev.second;
    
        data_tree->Fill();
    
        // the safest (?) way to update 
        // the file in the online mode
        if (online) { 
            data_tree->Write();
            tree_file->SaveSelf();
        }
    }
}

void StaveTTreeDataCollectorTLUSync::DoReset() {
    if (configured) {
        data_tree->Write();
        tree_file->Close();
    } 
    configured = false;
}

void StaveTTreeDataCollectorTLUSync::DoTerminate() {
    if (configured) {
        data_tree->Write();
        tree_file->Close();
    } 
    configured = false;
}
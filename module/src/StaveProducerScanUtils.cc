#include "StaveProducer.h"

void StaveProducer::init_scan(std::map<int,scan_info> scan_inf, scan_type st, 
                              int &max_it, int &row_0, int &max_it_reg, 
                              bool &it_global){
    // disable chips if they're not scanned
    for(auto si : scan_inf){
        if(!si.second.do_scan[st]){
            chips[si.first]->SetEnable(false);
            EUDAQ_INFO("DISABLED " + std::to_string(chips[si.first]->GetConfig()->GetChipId()));
        }
    }
    // configure chips
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Configuring ALPIDEs");
    int strobe_del_board = 20;
    int pulse_del_board  = 10000;
    int strobe_dur       = 80;
    int strobe_del_chip  = 0;
    int pulse_dur        = 500;
    mosaic->SendOpCode(Alpide::OPCODE_GRST);
    mosaic->SendOpCode(Alpide::OPCODE_PRST);
    for(auto si : scan_inf){
        if(!si.second.do_scan[st]) continue;
        AlpideConfig::BaseConfig(chips[si.first]);
        int pulse_type = 0x0;
        if(st == thr_dv || st == thr_ithr) pulse_type = 0x20;
        chips[si.first]->WriteRegister(Alpide::REG_FROMU_CONFIG1,  pulse_type);
        chips[si.first]->WriteRegister(Alpide::REG_FROMU_CONFIG2,  strobe_dur);
        chips[si.first]->WriteRegister(Alpide::REG_FROMU_PULSING1, strobe_del_chip); 
        chips[si.first]->WriteRegister(Alpide::REG_FROMU_PULSING2, pulse_dur);
        AlpideConfig::WritePixRegAll(chips[si.first], Alpide::PIXREG_MASK,   false);
        AlpideConfig::WritePixRegAll(chips[si.first], Alpide::PIXREG_SELECT, false);
        AlpideConfig::ConfigureCMUDMU(chips[si.first]);
        AlpideConfig::ConfigureDACs(chips[si.first]);
    }
    mosaic->SendOpCode(Alpide::OPCODE_RORST);
    bool pulse = (st != fhr);
    mosaic->SetTriggerConfig(pulse, true, strobe_del_board, pulse_del_board);
    mosaic->SetTriggerSource(trigInt);
    // setup iterations
    max_it = 0;
    for(auto si : scan_inf){
        if(!si.second.do_scan[st]) continue;
        if(max_it < si.second.n_it[st]) max_it = si.second.n_it[st];
    }
    // setup test mode
    row_0 = 0;
    for(auto si : scan_inf){
        if(!si.second.do_scan[st]) continue;
        if(si.second.test_mode[st]){
            row_0 = 511;
            break;
        }
    }
    // next -- threshold-specific
    if(st != thr_ithr && st != thr_dv){
        max_it_reg = -1;
        it_global  = false;
        return; 
    }
    // setup register iterations
    max_it_reg = 0;
    it_global  = true;
    std::vector<int> vals = (*scan_inf.begin()).second.ithr_range;
    for(auto si : scan_inf){
        if(st == thr_ithr && max_it_reg < si.second.ithr_range.size()) max_it_reg = si.second.ithr_range.size();
        if(st == thr_dv   && max_it_reg < si.second.ithr_range.size()) max_it_reg = si.second.dv_range.size();
        
        if(st == thr_ithr && vals.size() - si.second.ithr_range.size() == 0) vals = si.second.ithr_range;
        else if(st == thr_ithr) it_global  = false;
        if(st == thr_dv && vals.size() - si.second.ithr_range.size() == 0) vals = si.second.ithr_range;
        else if(st == thr_dv) it_global  = false;
    }
}

void StaveProducer::readout_stave(int n_chips, std::map<int,std::vector<std::tuple<int,int>>> &return_hits){
  // poll all of the active chips
  for(int i = 0; i < n_chips; i++){
    // setup buffers to store scan data
    unsigned char buffer[MAX_EVENT_SIZE];
    int n_bytes_data, n_bytes_header, n_bytes_trailer, prio_err = 0;
    TBoardHeader  board_info;
    std::vector<TPixHit>* hits = new std::vector<TPixHit>;
    int res = mosaic->ReadEventData(n_bytes_data, buffer);
    // discard no data events
    // (MOSAIC header is 64 bytes + 1 for the trailer + 1 
    // for EMPTY FRAME of ALPIDE)
    if(res <= 0 || n_bytes_data <= 66) continue;
    // decode MOSAIC event
    BoardDecoder::DecodeEventMOSAIC(buffer, n_bytes_data,
                                    n_bytes_header, n_bytes_trailer, 
                                    board_info);
    int recv = board_info.channel;
    // decode Chip event
    int n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
    int chip_id = -1;
    unsigned int bunch_counter = 0;
    int flags;
    bool decode = AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, 
                                             hits, 0, board_info.channel, prio_err, flags,
                                             40000000, 0x0, &chip_id, &bunch_counter);
    for(auto hit : *hits){
      int rem = hit.address % 4;
      if(rem == 0 || rem == 3){
          rem = 0;
      }
      else{
          rem = 1;
      }
      int pix_x = hit.region*32 + hit.dcol*2 + rem;
      int pix_y = hit.address/2;
      return_hits[chip_id].push_back(std::make_tuple(pix_x,pix_y));
    }
    delete hits;
  }
}
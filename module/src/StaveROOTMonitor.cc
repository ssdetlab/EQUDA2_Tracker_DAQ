#include "StaveROOTMonitor.h"

// TODO:: proper initialization. Now chip_id-unsafe

void StaveROOTMonitor::AtConfiguration(){
  auto conf = GetConfiguration();
  
  nof_staves = conf->Get("NOF_STAVES", 1); 
  for(int i = 0; i < nof_staves; i++){
    int nof_chips = conf->Get("NOF_CHIPS_" + std::to_string(i), 0); 

    for(int j = 0; j < nof_chips; j++){
      std::string hist_name  = "Pixel Grid, Stave " + std::to_string(i) + ", Chip " + std::to_string(j);
      std::string hist_title = "Pixel Grid, Stave " + std::to_string(i) + ", Chip " + std::to_string(j) + ";n_pix_x;n_pix_y";
      std::string hist_dir   = "Stave_" + std::to_string(i) + "/Pixel Grid, Stave " + std::to_string(i) + ", Chip " + std::to_string(j);

      pix_grid[i].push_back(m_monitor->Book<TH2I>(hist_dir.c_str(), 
                                                  hist_name.c_str(),
                                                  hist_name.c_str(), 
                                                  hist_title.c_str(),
                                                  128, 0, 1024, 64, 0, 512));
      m_monitor->SetDrawOptions(pix_grid[i][j], "colz");
    }
  }
}

void StaveROOTMonitor::AtEventReception(eudaq::EventSP ev){
  // size holders and counters
  int n_bytes_header, n_bytes_trailer;
  int n_corrupted_chip = 0, prio_err = 0;

  int st_id = std::stoi(ev->GetTag("Stave ID"));

  // iterate over the chips data
  std::vector<unsigned char> block = ev->GetBlock(st_id);
  // distribute frame decoding between several threads
  #pragma omp parallel for num_threads(block.size()/MAX_EVENT_SIZE)
  for(int i = 0; i < block.size()/MAX_EVENT_SIZE; i ++){
    // each chip frame is in a separate 
    // MAX_EVENT_SIZE block of data
    unsigned char buffer[MAX_EVENT_SIZE];
    for(int j = 0; j < MAX_EVENT_SIZE; j++){
      buffer[j] = block[i*MAX_EVENT_SIZE + j];
    }
    // extract frame size for the MOSAIC decoder
    int n_bytes_data; 
    if(ev->HasTag("Event size " + std::to_string(i))){
      n_bytes_data = std::stoi(ev->GetTag("Event size " + std::to_string(i)));
    }
    // decode MOSAIC event
    TBoardHeader board_info;
    BoardDecoder::DecodeEventMOSAIC(buffer, n_bytes_data, n_bytes_header, 
                                    n_bytes_trailer, board_info);
    // decode Chip event
    std::vector<TPixHit>* hits = new std::vector<TPixHit>;
    int n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
    int alpide_flags;
    bool not_corrrupted = AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, hits, st_id,
                                                     board_info.channel, prio_err, alpide_flags,
                                                     40000000);
    // convert hits to a more convenient
    // data structure
    int chip_id = (*hits)[0].chipId;
    int channel = (*hits)[0].channel;
    // decode region->double_column->address
    // data structure into a hit-map
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
      #pragma omp critical
      {
          pix_grid[st_id][chip_id]->Fill(pix_x, pix_y);
      }
    }
  }
}
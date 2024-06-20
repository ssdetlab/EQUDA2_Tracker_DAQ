#include "StaveFakeHitRateScanner.hh"

#include "TLUPayloadDecoder.h"

#include "json/writer.h"
#include "json/reader.h"
#include "json/value.h"

#include "AlpideConfig.h"
#include "TGraphErrors.h"

#include "TMath.h"

void StaveFakeHitRateScanner::prepare_scan() {
    int strobe_del_board = 20;
    int pulse_del_board  = 4000;
    int strobe_dur       = 80;
    int strobe_del_chip  = 20;
    int pulse_dur        = 500;
    int pulse_type       = 0x0;
    for (auto& [id,chip] : stave->chips) {
        if (!chip->GetConfig()->IsEnabled()) { 
            continue;
        }
        hit_map[id] = {};

        chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  pulse_type);
        chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  strobe_dur);
        chip->WriteRegister(Alpide::REG_FROMU_PULSING1, strobe_del_chip); 
        chip->WriteRegister(Alpide::REG_FROMU_PULSING2, pulse_dur);
        AlpideConfig::ConfigureCMUDMU(chip);
        AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK,   false);
        AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);
    }
    stave->mosaic->SendOpCode(Alpide::OPCODE_RORST);
    stave->mosaic->SetTriggerConfig(false, true, strobe_del_board, pulse_del_board);
    stave->mosaic->SetTriggerSource(trigInt);
}

void StaveFakeHitRateScanner::fhr_scan(
    std::string path, int n_it) {
        std::cout << 
            "Stave_"  <<
            stave->stave_id <<
            ": Starting fake hit rate ithr scan" << std::endl;
        std::cout <<
            "Stave_" << 
            stave->stave_id << 
            ": Configuring ALPIDEs" << std::endl;

        for (auto& [id,chip] : stave->chips) {
            if (!chip->GetConfig()->ScanFakeHitRate()) {
                chip->SetEnable(false);
            }
        }
        std::cout << "Preparing scan" << std::endl;
        prepare_scan();

        std::cout << "Starting iterations" << std::endl;
        stave->mosaic->StartRun();
        for (int i = 0; i < n_it; i++) {
            stave->mosaic->Trigger(1);
            // poll all of the active chips
            for (int i = 0; i < hit_map.size(); i++) {
                // setup buffers to store scan data
                unsigned char buffer[MAX_EVENT_SIZE];
                int n_bytes_data;
                TBoardHeader  board_info;
                std::vector<TPixHit> hits;
    
                int res = stave->mosaic->ReadEventData(n_bytes_data, buffer);

                // discard no data events
                if ((buffer[64] & 0xf0) == 0xe0) {
                    continue;
                }
                int n_bytes_header, n_bytes_trailer, prio_err = 0;
                // decode MOSAIC event
                BoardDecoder::DecodeEventMOSAIC(
                    buffer, n_bytes_data,
                    n_bytes_header, n_bytes_trailer, 
                    board_info);
    
                std::uint16_t tlu_trig_n;
                int n_bytes_tlu;
                if(TLUPayloadDecoder::DecodeEvent(
                    buffer + 
                    n_bytes_data - 
                    TLUPayloadDecoder::DSIZE_TLU_PAYLOAD - 
                    n_bytes_trailer, tlu_trig_n)) {
                        n_bytes_tlu = TLUPayloadDecoder::DSIZE_TLU_PAYLOAD;
                } else {
                    n_bytes_tlu = 0;
                }

                // decode Chip event
                int n_bytes_chipevent = 
                    n_bytes_data - 
                    n_bytes_header - 
                    n_bytes_trailer -
                    n_bytes_tlu;
                int chip_id = -1;
                unsigned int bunch_counter = 0;
                int flags;
                bool decode = AlpideDecoder::DecodeEvent(
                    buffer + n_bytes_header, n_bytes_chipevent, 
                    &hits, 0, board_info.channel, prio_err, flags,
                    40000000, 0x0, &chip_id, &bunch_counter);
                
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
                    hit_map[chip_id].push_back(std::make_pair(pix_x,pix_y));
                }
            }
            usleep(10);
        }
        stave->mosaic->StopRun();

        std::fstream fin;
        path += "stave_" + std::to_string(stave->stave_id) + ".json";
        fin.open(path, std::ios::in);

        Json::Value root;
        Json::Reader reader;
        reader.parse(fin, root);
        
        Json::StyledStreamWriter writer;
        for (auto [id,hits] : hit_map) {
            Json::Value pix_x(Json::arrayValue), pix_y(Json::arrayValue); Json::Value prob(Json::arrayValue);
            int count;
            for(auto hit : hits){
                int ix = hit.first;
                int iy = hit.second;
                if (std::count(pix_x.begin(), pix_x.end(), ix) == 0 ||
                    std::count(pix_y.begin(), pix_y.end(), iy) == 0) {
                    count = std::count(hits.begin(), hits.end(), hit);
                    pix_x.append(ix); 
                    pix_y.append(iy); 
                    prob.append(float(count)/n_it);
                }
            }
            root[std::to_string(id)]["bad_pixels"]["pix_x"] = pix_x;
            root[std::to_string(id)]["bad_pixels"]["pix_y"] = pix_y;
            root[std::to_string(id)]["bad_pixels"]["prob"]  = prob;
        }

        std::cout << root << std::endl;

        std::fstream fout;
        fout.open(path, std::ios::out);

        writer.write(fout, root);
        fout.close();

        for (auto& [id,chip] : stave->chips) {
            chip->SetEnable(true);
        }
}
#include "StaveProducer.h"

void StaveProducer::fhr_scan(std::string dir, std::map<int,scan_info> scan_inf){
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": starting fake hit rate scan");
    int max_it, row_0, dummy_1;
    bool dummy_2;
    init_scan(scan_inf, fhr, max_it, row_0, dummy_1, dummy_2);
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": starting fake hit rate scan");
    std::map<int,std::vector<std::tuple<int,int>>> total_hits;
    int active = 0;
    for(auto si : scan_inf){
        if(si.second.do_scan[fhr]) active++;
    }
    int it = 0;
    mosaic->StartRun();
    while(it < max_it){
        mosaic->Trigger(1);
        readout_stave(active, total_hits);
        if(it%1000 == 0) EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": iteration " + std::to_string(it) + " DONE");
        it++;
        // check for chips to disable
        for(auto si : scan_inf){
            if(!si.second.do_scan[fhr]) continue;
            if(it >= si.second.n_it[fhr] && chip_confs[si.first]->IsEnabled()){
                mosaic->StopRun();
                chips[si.first]->SetEnable(false);
                EUDAQ_INFO("DISABLED " + std::to_string(si.first));
                mosaic->StartRun();
                active--;
            }
        }
    }
    mosaic->StopRun();
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": writing data into a file");
    // enable chips back
    for(auto chip : chips){
        chip.second->SetEnable(true);
    }
    // store scan results
    try{
        // check if telescope_stats already exists to append new data
        Json::Value root;
        Json::Reader reader;
        std::ifstream fin;
        std::string path = dir + "stave_stats_" + std::to_string(this->stave_id) + ".json";
        fin.open(path);
        reader.parse(fin, root);
        fin.close();
        // write bad pixels and probabilities into json file
        Json::StyledStreamWriter writer;
        for(auto data : total_hits){
            Json::Value pix_x(Json::arrayValue), pix_y(Json::arrayValue); Json::Value prob(Json::arrayValue);
            int count;
            for(auto hit : data.second){
                int ix = std::get<0>(hit), iy = std::get<1>(hit);
                if(std::count(pix_x.begin(), pix_x.end(), ix) == 0 ||
                    std::count(pix_y.begin(), pix_y.end(), iy) == 0){
                    count = std::count(data.second.begin(), data.second.end(), hit);
                    pix_x.append(ix); pix_y.append(iy); prob.append(float(count)/scan_inf[data.first].n_it[fhr]);
                }
                root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"]["fake_hit_rate"]["pix_x"] = pix_x;
                root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"]["fake_hit_rate"]["pix_y"] = pix_y;
                root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"]["fake_hit_rate"]["prob"]  = prob;
            }
        }
        std::ofstream fout;
        fout.open(path);
        writer.write(fout, root);
        fout.close();
    }
    catch(const std::exception &e){
        EUDAQ_WARN(e.what());
    }
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": finished fake hit rate scan");
}

void StaveProducer::digital_scan(std::string dir, std::map<int,scan_info> scan_inf){
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": starting digital scan");
    int max_it, row_0, dummy_1;
    bool dummy_2;
    init_scan(scan_inf, digital, max_it, row_0, dummy_1, dummy_2);
    // chip->meb->hit->pix_x/pix_y
    std::map<int,std::vector<std::tuple<int,int>>> stuck;
    std::map<int,std::vector<std::tuple<int,int>>> unmaskable;
    int active = 0;
    for(auto si : scan_inf){
        if(si.second.do_scan[digital]) active++;
    }
    int it = 0;
    mosaic->StartRun();
    while(it < max_it){
        for(int row = row_0; row < 512; row++){
            for(bool pulse : {true, false}){
                mosaic->SetTriggerConfig(pulse, true, 20, 10000);
                // mask everything except one row
                for(auto si : scan_inf){
                    if(!si.second.do_scan[digital]) continue;
                    AlpideConfig::ConfigureMaskStage(chips[si.first], 32, row);
                }
                std::map<int,std::vector<std::tuple<int,int>>> temp;
                mosaic->Trigger(1);
                readout_stave(active, temp);
                for(auto data : temp){
                    for(auto hit : data.second){
                        if(pulse && std::get<1>(hit) != row) unmaskable[data.first].push_back(hit);
                        else if(!pulse) stuck[data.first].push_back(hit);
                    }
                }
            }
            if(row%32 == 0) EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": row " + std::to_string(row) + " iteration " + std::to_string(it) + " DONE");
        }
        it++;
        // check for chips to disable
        for(auto si : scan_inf){
            if(!si.second.do_scan[digital]) continue;
            if(it >= si.second.n_it[digital] && chip_confs[si.first]->IsEnabled()){
                mosaic->StopRun();
                chips[si.first]->SetEnable(false);
                EUDAQ_INFO("DISABLED " + std::to_string(si.first));
                mosaic->StartRun();
                active--;
            }
        }
    }
    mosaic->StopRun();
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": writing data into a file");
    // enable chips back
    for(auto chip : chips){
        chip.second->SetEnable(true);
    }
    // store scan results
    try{
        // check if telescope_stats already exists to append new data
        Json::Value root;
        Json::Reader reader;
        std::ifstream fin;
        std::string path = dir + "stave_stats_" + std::to_string(this->stave_id) + ".json";
        fin.open(path);
        reader.parse(fin, root);
        fin.close();
        // write bad pixels and probabilities into json file
        Json::StyledStreamWriter writer;
        std::vector<std::string> names = {"stuck", "unmaskable"};
        int k = 0;
        for(auto scan_res : {stuck, unmaskable}){
            for(auto data : scan_res){
                Json::Value pix_x(Json::arrayValue), pix_y(Json::arrayValue); Json::Value prob(Json::arrayValue);
                int count;
                for(auto hit : data.second){
                    int ix = std::get<0>(hit), iy = std::get<1>(hit);
                    if(std::count(pix_x.begin(), pix_x.end(), ix) == 0 ||
                        std::count(pix_y.begin(), pix_y.end(), iy) == 0){
                        count = std::count(data.second.begin(), data.second.end(), hit);
                        pix_x.append(ix); pix_y.append(iy); prob.append(float(count)/scan_inf[data.first].n_it[digital]);
                    }
                    root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"][names[k]]["pix_x"] = pix_x;
                    root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"][names[k]]["pix_y"] = pix_y;
                    root[std::to_string(data.first) + "," + std::to_string(chips[data.first]->GetConfig()->GetDataLink())]["bad_pixels"][names[k]]["prob"]  = prob;
                }
            }
            k++;
        }
        std::ofstream fout;
        fout.open(path);
        writer.write(fout, root);
        fout.close();
    }
    catch(const std::exception &e){
        EUDAQ_WARN(e.what());
    }
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": finished digital scan");
}

void StaveProducer::config_bad_pixels(std::string dir, double stuck_tol, double fhr_tol){
    // read json stats
    Json::Value root;
    Json::Reader reader;
    std::ifstream fin;
    std::string path = dir + "stave_stats_" + std::to_string(this->stave_id) + ".json";
    fin.open(path);
    reader.parse(fin, root);
    // mask stuck pixels to minimize their presence in the data 
    for(auto chip : chips){
        int chip_id   = chip.second->GetConfig()->GetChipId();
        int chip_link = chip.second->GetConfig()->GetDataLink();
        Json::Value stuck_pix_x = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["stuck"]["pix_x"];
        Json::Value stuck_pix_y = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["stuck"]["pix_y"];
        Json::Value stuck_prob  = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["stuck"]["prob"];       
        for(int i = 0; i < stuck_pix_x.size(); i++){
          int ix    = stuck_pix_x[i].asInt();
          int iy    = stuck_pix_y[i].asInt(); 
          double pr = stuck_prob[i].asDouble();
          if(pr >= stuck_tol){
            AlpideConfig::WritePixRegSingle(chip.second, Alpide::PIXREG_MASK, true, iy, ix);
          }
        }
    }
    // mask pixels with unacceptable fake hit rate
    for(auto chip : chips){
        int chip_id   = chip.second->GetConfig()->GetChipId();
        int chip_link = chip.second->GetConfig()->GetDataLink();
        Json::Value fhr_pix_x = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["fake_hit_rate"]["pix_x"];
        Json::Value fhr_pix_y = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["fake_hit_rate"]["pix_y"];
        Json::Value fhr_prob  = root[std::to_string(chip_id) + "," + std::to_string(chip_link)]["bad_pixels"]["fake_hit_rate"]["prob"];
        for(int i = 0; i < fhr_pix_x.size(); i++){
          int ix    = fhr_pix_x[i].asInt();
          int iy    = fhr_pix_y[i].asInt(); 
          double pr = fhr_prob[i].asDouble();
          if(pr >= fhr_tol){
            AlpideConfig::WritePixRegSingle(chip.second, Alpide::PIXREG_MASK, true, iy, ix);
          }
        }
    }
    fin.close();
}
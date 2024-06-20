#include "StaveThresholdScanner.hh"
#include "TLUPayloadDecoder.h"

#include "AlpideConfig.h"
#include "TGraphErrors.h"

#include "TMath.h"

// function to fit pixel's threshold curve
double thr_fit_func(double* x, double* par) {
    double _x  = x[0];
    double thr = par[0]; // threshold
    double ns  = par[1]; // noise
  
    // assume gaussian electrical noise
    double val = 0.5*(1 + TMath::Erf((_x - thr)/(TMath::Sqrt(2)*ns)));
    return val;
}

// function to fit pixel's threshold curve
double _thr_fit_func(double* x, double* par) {
    double _x  = x[0];
    double thr = par[0]; // threshold
    double ns  = par[1]; // noise
    
    // assume gaussian electrical noise
    double val = 0.5*(1 + TMath::Erf((-_x + thr)/(TMath::Sqrt(2)*ns)));
    return val;
}

void StaveThresholdScanner::prepare_scan(  
    std::map<int,std::vector<
        std::pair<int,double>>>& data_to_init) {
            int strobe_del_board = 20;
            int pulse_del_board  = 4000;
            int strobe_dur       = 80;
            int strobe_del_chip  = 20;
            int pulse_dur        = 500;
            int pulse_type       = 0x20;
            for (auto& [id,chip] : stave->chips) {
                if (!chip->GetConfig()->IsEnabled()) { 
                    continue;
                }
                data_to_init[id] = {};
        
                chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  pulse_type);
                chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  strobe_dur);
                chip->WriteRegister(Alpide::REG_FROMU_PULSING1, strobe_del_chip); 
                chip->WriteRegister(Alpide::REG_FROMU_PULSING2, pulse_dur);
                AlpideConfig::ConfigureCMUDMU(chip);
                AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK,   false);
                AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);
            }
            stave->mosaic->SendOpCode(Alpide::OPCODE_RORST);
            stave->mosaic->SetTriggerConfig(true, true, strobe_del_board, pulse_del_board);
            stave->mosaic->SetTriggerSource(trigInt);
}

#include <chrono>

void StaveThresholdScanner::iter_reg(
    Alpide::TRegister reg, 
    std::vector<int> range,
    int n_it,
    std::map<int,std::vector<
        std::pair<int,double>>>& data) {
            for (int val : range) {
                std::cout << val << std::endl;
                std::map<int, std::vector<int>> hit_map;
                for (auto& [id,chip] : stave->chips) {
                    if (!chip->GetConfig()->IsEnabled()) { 
                            continue;
                    }
                    stave->mosaic->WriteChipRegister(
                        reg, val, chip);
                    hit_map[id] = {};
                }
                for (int row = 0; row < 512; row++) {
                    std::cout << "ROW = " << row << std::endl;
                    for (auto& [id,chip] : stave->chips) {
                        if (!chip->GetConfig()->IsEnabled()) { 
                            continue;
                        }
                        AlpideConfig::ConfigureMaskStage(chip, 32, row);
                    }
                    stave->mosaic->Trigger(n_it);
                    // poll all of the active chips
                    for (int i = 0; i < n_it*hit_map.size(); i++) {
                        // setup buffers to store scan data
                        unsigned char buffer[MAX_EVENT_SIZE];
                        int n_bytes_data;
                        TBoardHeader  board_info;
                        std::vector<TPixHit> hits;

                        auto start = std::chrono::high_resolution_clock::now();
                        int res = stave->mosaic->ReadEventData(n_bytes_data, buffer);
                        auto stop = std::chrono::high_resolution_clock::now();

                        std::cout << "READOUT DURATION = " << (stop - start).count() << std::endl;

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
                        hit_map[chip_id].push_back(hits.size());
                    }
                }
                for (auto& [id,hits] : hit_map) {
                    double tot_hits = 0;
                    for (auto hit : hits) {
                        tot_hits += hit;
                    }
                    std::cout << val << " " << tot_hits/(1024*512*n_it) << std::endl; 
                    data.at(id).push_back(
                        std::make_pair(val,tot_hits/(1024*512*n_it)));
                }
            }
}

std::pair<double,double> 
StaveThresholdScanner::fit_thr_curve(
    std::vector<std::pair<int,double>> reg_to_hits, 
    TF1& fit_func,
    std::string path) {
        // number of hits in each step 
        // follows the Binomial distribution
        TGraphErrors thr_gr;
        int j = 0;
        for (auto rth : reg_to_hits) {
            double p = rth.second;
            thr_gr.SetPoint(j, rth.first, rth.second);
            if (p != 0 && (1 - p) != 0) {
                thr_gr.SetPointError(j, 0, TMath::Sqrt(p*(1 - p)/5));
            }
            else { 
                thr_gr.SetPointError(j, 0, 1);
            }
            j++;
        }
        thr_gr.Fit(&fit_func);
        if (!path.empty()) {
            thr_gr.SaveAs(path.c_str());
        }

        return std::make_pair(
            fit_func.GetParameter(0), fit_func.GetParameter(1));
}

std::pair<int,int> 
StaveThresholdScanner::precheck_range(Alpide::TRegister reg, int min, int max) {
    std::vector<int> precheck_range = {min, max};

    std::map<int,std::vector<
        std::pair<int,double>>> precheck_map;
    for (auto& [id,chip] : stave->chips) {
        if (!chip->GetConfig()->IsEnabled()) { 
            continue;
        }
        precheck_map[id] = {};
    }

    iter_reg(reg, precheck_range, 10, precheck_map);

    std::pair<int,int> res{0,0};
    for (auto& [id,data] : precheck_map) {
        if (data.at(0).second > 0.1) {
            res.first = 1;
        }
        if(data.at(1).second < 0.9) {
            res.second = 1;
        }
    }
    return res;
}

std::map<int,int> 
StaveThresholdScanner::ithr_scan(
    std::string path,
    int dv_thr, 
    int n_it,
    int ithr_min,
    int ithr_max) {

        std::cout << 
            "Stave_"  <<
            stave->stave_id <<
            ": Starting threshold ithr scan" << std::endl;
        std::cout <<
            "Stave_" << 
            stave->stave_id << 
            ": Configuring ALPIDEs" << std::endl;

        for (auto& [id,chip] : stave->chips) {
            if (!chip->GetConfig()->ScanThresholdIthr()) {
                chip->SetEnable(false);
            }
            else {
                stave->mosaic->WriteChipRegister(Alpide::REG_VPULSEH, 170, chip);
                stave->mosaic->WriteChipRegister(Alpide::REG_VPULSEL, 170 - dv_thr, chip);
            }
        }

        prepare_scan(ithr_map);

        std::cout << 
            "Stave_" <<
            stave->stave_id <<
            ": Iterating over the rows" << std::endl;

        stave->mosaic->StartRun();
        while (true) {
            auto res = precheck_range(Alpide::REG_ITHR, ithr_max, ithr_min);
            if (res.first == 0 && res.second == 0) {
                break;
            } else {
                if (res.first == 1) {
                    ithr_max += 5;
                }
                if (res.second == 1) {
                    ithr_min -= 5;
                }
            }
        }
        stave->mosaic->StopRun();


        std::vector<int> ithr_range;
        for (int ithr = ithr_min; ithr < ithr_max; ithr++) {
            ithr_range.push_back(ithr);
        }

        stave->mosaic->StartRun();
        iter_reg(Alpide::REG_ITHR, ithr_range, n_it, ithr_map);
        stave->mosaic->StopRun();

        // fit threshold curves
        std::map<int,int> ithr_res;
        for (auto& [id,data] : ithr_map) {
            auto curve = TF1("fitFunc", _thr_fit_func, ithr_min, ithr_max, 2);
            curve.SetParameters((ithr_max + ithr_min)/2., 1);
            std::string name = "ITHR_curve_" + std::to_string(id) + ".C";
            std::string file = path + name;
            auto [threshold,noise] = fit_thr_curve(data, curve, file);
            ithr_res[id] = static_cast<int>(std::round(threshold));
            std::cout << 
                "Chip " << 
                id << 
                ": ITHR = " << ithr_res[id] << std::endl;
        }
        std::cout << "Stave_" <<
            stave->stave_id << 
            ": Finished threshold ithr scan" << std::endl;

        for (auto& [id,chip] : stave->chips) {
            chip->SetEnable(true);
        }

        return ithr_res;
}

std::map<int,int> 
StaveThresholdScanner::dv_scan(
    std::string path, 
    int n_it,
    int dv_min,
    int dv_max) {
        std::cout <<
            "Stave_" << 
            stave->stave_id <<
            ": Starting threshold dv scan" << std::endl;
 
        for (auto& [id,chip] : stave->chips) {
            if (!chip->GetConfig()->ScanThresholdDv()) {
                chip->SetEnable(false);
            }
        }
 
        std::cout <<
            "Stave_" << 
            stave->stave_id <<
            ": Configuring ALPIDEs" << std::endl;
        prepare_scan(dv_map);

        std::cout << 
            "Stave_" + 
            stave->stave_id << 
            ": Iterating over the rows" << std::endl;

        stave->mosaic->StartRun();
        while(true) {
            auto res = precheck_range(Alpide::REG_VPULSEL, 170 - dv_min, 170 - dv_max);
            if (res.first == 0 && res.second == 0) {
                break;
            } else {
                if (res.first == 1) {
                    dv_min -= 5;
                }
                if (res.second == 1) {
                    dv_max += 5;
                }
            }
        }
        stave->mosaic->StopRun();

        std::vector<int> dv_range;
        for (int dv = dv_min; dv < dv_max; dv++) {
            dv_range.push_back(170 - dv);
        }

        stave->mosaic->StartRun();
        iter_reg(Alpide::REG_VPULSEL, dv_range, n_it, dv_map);
        stave->mosaic->StopRun();
   
        // fit threshold curves
        std::map<int,int> dv_res;
        for (auto& [id,data] : dv_map) {
            auto curve = TF1("fitFunc", thr_fit_func, dv_min, dv_max, 2);
            curve.SetParameters((dv_max + dv_min)/2., 1);
            std::string name = "dv_curve_" + std::to_string(id) + ".C";
            std::string file = path + name;
            std::vector<std::pair<int,double>> shifted_data;
            for (auto entry : data) {
                shifted_data.push_back(std::make_pair(170 - entry.first, entry.second));
            }
            auto [threshold,noise] = fit_thr_curve(shifted_data, curve, file);
            dv_res[id] = static_cast<int>(std::round(threshold));
            std::cout << 
                "Chip " << 
                id  << 
                ": DV = " << dv_res[id] << std::endl;
        }
        std::cout << "Stave_" <<
            stave->stave_id << 
            ": Finished threshold dv scan" << std::endl;
 
        for (auto& [id,chip] : stave->chips) {
            chip->SetEnable(true);
        }

        return dv_res;
}
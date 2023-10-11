#include "StaveProducer.h"

// TODO:: fix memory (?) issues with threshold scan

// function to fit pixel's threshold curve
double thr_fit_func(double* x, double* par){
  double _x  = x[0];
  double a   = par[0]; // normalization
  double thr = par[1]; // threshold
  double ns  = par[2]; // noise
  
  // assume gaussian electrical noise
  double val = (a/2)*(1 + TMath::Erf((_x - thr)/(TMath::Sqrt(2)*ns)));
  return val;
}

// function to fit pixel's threshold curve
double _thr_fit_func(double* x, double* par){
  double _x  = x[0];
  double a   = par[0]; // normalization
  double thr = par[1]; // threshold
  double ns  = par[2]; // noise
  
  // assume gaussian electrical noise
  double val = (a/2)*(1 + TMath::Erf((-_x + thr)/(TMath::Sqrt(2)*ns)));
  return val;
}

void StaveProducer::threshold_scan(std::string path, std::map<int,scan_info> scan_inf){
    // find adequate ITHR values for given threshold
    std::string ithr_path = path + "stave_" + std::to_string(this->stave_id) + "/thr_scans/ithr_scan.root";
    std::map<int,int> new_ithrs = threshold_ithr_scan(ithr_path, scan_inf);
    // // apply new ITHR values
    // for(auto si : scan_inf){
        // if(si.second.do_scan[thr_ithr]){
            // mosaic->WriteChipRegister(Alpide::REG_ITHR, new_ithrs[si.first], chips[si.first]);
        // }
    // }
    // // find new thresholds
    // std::string dv_path = path + "stave_" + std::to_string(this->stave_id) + "/thr_scans/dv_scan.root";
    // std::map<int,int> new_thrs = threshold_dv_scan(dv_path, scan_inf);
}

void StaveProducer::init_thr_analysis(std::map<int,scan_info>     scan_inf,
                                      scan_type                   st,
                                      std::map<int,TF1*>          &fit_funcs, 
                                      std::map<int,TGraphErrors*> &thr_grs, 
                                      std::map<int,TH1D*>         &thr_hists, 
                                      std::map<int,TH1D*>         &noise_hists){
    for(auto si : scan_inf){
        if(!si.second.do_scan[st]) continue;
        int chip_id = chips[si.first]->GetConfig()->GetChipId();
        int bound_min;
        int bound_max;
        if(st == thr_ithr){
            bound_min = *(si.second.ithr_range.begin());
            bound_max = *(si.second.ithr_range.end() - 1);
        }
        else if(st == thr_dv){
            bound_min = *(si.second.dv_range.begin());
            bound_max = *(si.second.dv_range.end() - 1);
        }
        int n_it = si.second.n_it[st];
        std::string func_name = std::to_string(st) + "_fit_func_" + std::to_string(chip_id);
        if(st == thr_ithr) fit_funcs[si.first] = new TF1(func_name.c_str(), _thr_fit_func, bound_min, bound_max, 3);
        if(st == thr_dv)   fit_funcs[si.first] = new TF1(func_name.c_str(), thr_fit_func,  bound_min, bound_max, 3);
        fit_funcs[si.first]->SetParameters(n_it, (bound_max - bound_min)/2., 0.5);  
        fit_funcs[si.first]->FixParameter(0, n_it);
        fit_funcs[si.first]->SetParLimits(1, 0, 255);
        fit_funcs[si.first]->SetParLimits(2, 0.01, 255);

        std::string gr_name = std::to_string(st) + "_thr_gr_" + std::to_string(chip_id);
        thr_grs[chip_id]    = new TGraphErrors();
        thr_grs[chip_id]->SetNameTitle(gr_name.c_str(), gr_name.c_str());

        std::string name_thr  = std::to_string(st) + "_thist_" + std::to_string(chip_id);
        std::string name_thr_ = name_thr + ";Register Value, DAC Ch.; Entries";
        thr_hists[chip_id]    = new TH1D(name_thr.c_str(), name_thr_.c_str(), 10*(bound_max-bound_min), bound_min, bound_max);

        std::string name_noise  = std::to_string(st) + "_nhist_" + std::to_string(chip_id);
        std::string name_noise_ = name_thr + ";Register Value, DAC Ch.; Entries";
        noise_hists[chip_id]    = new TH1D(name_noise.c_str(), name_noise_.c_str(), 1000, 0, 100);
    }    
}

bool StaveProducer::fit_thr_curve(int n_it,
                                  std::map<int,int> reg_to_hits, 
                                  TF1*              fit_func, 
                                  TGraphErrors*     thr_gr, 
                                  TH1D*             thr_hist, 
                                  TH1D*             noise_hist){
    // number of hits in each step follows the Binomial distribution
    int j = 0;
    for(auto rth : reg_to_hits){
        double p = static_cast<double>(rth.second)/n_it;
        thr_gr->SetPoint(j, rth.first, rth.second);
        if(p != 0 && (1 - p) != 0) thr_gr->SetPointError(j, 0, TMath::Sqrt(n_it*p*(1 - p)));
        else                       thr_gr->SetPointError(j, 0, 1);
        j++;
    }
    TFitResultPtr res_p = thr_gr->Fit(fit_func, "QRS");
    TFitResult*   res   = res_p.Get();
    // discard unprobable results
    if(res->Prob() > 0.05){
        thr_hist->Fill(fit_func->GetParameter(1));
        noise_hist->Fill(fit_func->GetParameter(2));
        return true;
    }
    else{
        return false;
    }  
}

std::map<int,int> StaveProducer::threshold_ithr_scan(std::string path, std::map<int,scan_info> scan_inf){
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Starting threshold ithr scan");
    int max_it, row_0, max_ithr_it;
    bool it_global;
    init_scan(scan_inf, thr_ithr, max_it, row_0, max_ithr_it, it_global);
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Setting up functions and histograms");
    std::map<int,TF1*>          fit_funcs;
    std::map<int,TGraphErrors*> thr_grs;
    std::map<int,TH1D*>         thr_hists;
    std::map<int,TH1D*>         noise_hists;
    init_thr_analysis(scan_inf, thr_ithr, fit_funcs, thr_grs, thr_hists, noise_hists);
    std::map<int,std::vector<std::tuple<int,int>>> dead;
    std::map<int,std::vector<std::tuple<int,int>>> bad;
    // iterate over the matrix rows 
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Iterating over the rows");
    mosaic->StartRun();
    for(int row = row_0; row < 512; row++){
        int ithr_it = 0;
        // chip->reg_val->x->n_hits 
        std::map<int,std::map<int,std::vector<int>>> row_hits;
        for(auto si : scan_inf){
            int chip_id = si.first;
            for(int rv = 0; rv < si.second.ithr_range.size(); rv++){
                row_hits[chip_id][scan_inf[chip_id].ithr_range[rv]].resize(1024, 0);
            } 
        }
        // mask everything except one row
        for(auto id : scan_inf){
            AlpideConfig::ConfigureMaskStage(chips[id.first], 32, row);
        }
        while(ithr_it < max_ithr_it){
            int it = 0, active = scan_inf.size();
            // set ITHR registers
            for(auto chip : chips){
                mosaic->WriteChipRegister(Alpide::REG_ITHR,  
                                          scan_inf[chip.second->GetConfig()->GetChipId()].ithr_range[ithr_it], 
                                          chip.second);
            }
            while(it < max_it){
                mosaic->Trigger(1);
                std::map<int,std::vector<std::tuple<int,int>>> temp;
                readout_stave(active, temp);
                for(auto data : temp){
                    for(auto hit : data.second){
                        if(std::get<1>(hit) == row) row_hits[data.first][scan_inf[data.first].ithr_range[ithr_it]][std::get<0>(hit)]++;
                    }
                }
                it++;
                if(it_global) continue;
                // check for chips to disable 
                // if different amount of iterations
                for(auto si : scan_inf){
                    if(!si.second.do_scan[thr_ithr]) continue;
                    if(it >= scan_inf[si.first].n_it[thr_ithr] && chip_confs[si.first]->IsEnabled()){
                        mosaic->StopRun();
                        chips[si.first]->SetEnable(false);
                        mosaic->StartRun();
                        active--;
                    }
                }
            }
            ithr_it++;
            // check for chips to enable back
            for(auto si : scan_inf){
                if(!si.second.do_scan[thr_ithr]) continue;
                if(ithr_it < si.second.ithr_range.size() && !chip_confs[si.first]->IsEnabled()){
                    mosaic->StopRun();
                    chips[si.first]->SetEnable(true);
                    mosaic->StartRun();
                    active--;
                }
            }
        }
        // fit threshold curves
        for(int i = 0; i < 1024; i++){
            for(auto si : scan_inf){
                if(!si.second.do_scan[thr_ithr]) continue;
                int max     = 0;
                int chip_id = si.first;
                int n_it    = scan_inf[chip_id].n_it[thr_ithr]; 
                std::vector<int>  reg_vals = si.second.ithr_range;
                std::map<int,int> reg_to_hits;
                // omit "dead" pixels
                for(int reg_val : reg_vals){
                    if(row_hits[chip_id][reg_val][i] > max) max = row_hits[chip_id][reg_val][i];
                    reg_to_hits[reg_val] = row_hits[chip_id][reg_val][i];
                }
                if(max == 0){
                    dead[chip_id].push_back(std::make_tuple(i,row));
                    continue;
                }
                if(!fit_thr_curve(n_it, reg_to_hits, 
                                  fit_funcs[chip_id], 
                                  thr_grs[chip_id], 
                                  thr_hists[chip_id], 
                                  noise_hists[chip_id])) bad[chip_id].push_back(std::make_tuple(i,row));
                int ithrmin = *(scan_inf[chip_id].ithr_range.begin());
                int ithrmax = *(scan_inf[chip_id].ithr_range.end() - 1);
                fit_funcs[chip_id]->SetParameters(n_it, (ithrmax - ithrmin)/2., 0.5);
                fit_funcs[chip_id]->FixParameter(0, n_it);
            }
        }
        EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Row: " + std::to_string(row) + " DONE");
        // enable chips back
        for(auto si : scan_inf){
            if(!si.second.do_scan[thr_ithr]) continue;
            chips[si.first]->SetEnable(true);
        }
    }
    mosaic->StopRun();
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": writing data into a file");
    // store scan results
    TFile* file = new TFile(path.c_str(), "recreate");
    std::map<int,int> ithr_res;
    for(auto si : scan_inf){
        if(!si.second.do_scan[thr_ithr]) continue;
        thr_hists[si.first]->SetLineColor(kBlue);
        thr_hists[si.first]->SetFillColor(kBlue);
        noise_hists[si.first]->SetLineColor(kBlue);
        noise_hists[si.first]->SetFillColor(kBlue);

        thr_hists[si.first]->Write();
        noise_hists[si.first]->Write();
        thr_grs[si.first]->Write("DebugGraph");

        ithr_res[si.first] = std::round(thr_hists[si.first]->GetMean());
    }
    file->Close();
    // enable chips back
    for(auto chip : chips){
        chip.second->SetEnable(true);
    }
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Finished threshold ithr scan");
    return ithr_res;
}

std::map<int,int> StaveProducer::threshold_dv_scan(std::string path, std::map<int,scan_info> scan_inf){
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Starting threshold dv scan");
    int max_it, row_0, max_dv_it;
    bool it_global;
    init_scan(scan_inf, thr_dv, max_it, row_0, max_dv_it, it_global);
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Setting up functions and histograms");
    std::map<int,TF1*>          fit_funcs;
    std::map<int,TGraphErrors*> thr_grs;
    std::map<int,TH1D*>         thr_hists;
    std::map<int,TH1D*>         noise_hists;
    init_thr_analysis(scan_inf, thr_dv, fit_funcs, thr_grs, thr_hists, noise_hists);
    std::map<int,std::vector<std::tuple<int,int>>> dead;
    std::map<int,std::vector<std::tuple<int,int>>> bad;
    // iterate over the matrix rows 
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Iterating over the rows");
    mosaic->StartRun();
    for(int row = 0; row < 10; row++){
        int dv_it = 0;
        // chip->reg_val->x->n_hits 
        std::map<int,std::map<int,std::vector<int>>> row_hits;
        for(auto si : scan_inf){
            int chip_id = si.first;
            for(int rv = 0; rv < si.second.dv_range.size(); rv++){
                row_hits[chip_id][scan_inf[chip_id].dv_range[rv]].resize(1024,0);
            } 
        }
        // mask everything except one row
        for(auto id : scan_inf){
            AlpideConfig::ConfigureMaskStage(chips[id.first], 32, row);
        }
        while(dv_it < max_dv_it){
            int it = 0, active = scan_inf.size();
            // set registers
            for(auto chip : chips){
                mosaic->WriteChipRegister(Alpide::REG_VPULSEL,  
                                          chip.second->GetConfig()->GetParamValue("VPULSEH") 
                                          - scan_inf[chip.second->GetConfig()->GetChipId()].dv_range[dv_it], 
                                          chip.second);
            }
            while(it < max_it){
                mosaic->Trigger(1);
                std::map<int,std::vector<std::tuple<int,int>>> temp;
                readout_stave(active, temp);
                for(auto data : temp){
                    for(auto hit : data.second){
                        if(std::get<1>(hit) == row) row_hits[data.first][scan_inf[data.first].dv_range[dv_it]][std::get<0>(hit)]++;
                    }
                }
                it++;
                if(it_global) continue;
                // check for chips to disable 
                // if different amount of iterations
                for(auto si : scan_inf){
                    if(!si.second.do_scan[thr_dv]) continue;
                    if(it >= scan_inf[si.first].n_it[thr_dv] && chip_confs[si.first]->IsEnabled()){
                        mosaic->StopRun();
                        chips[si.first]->SetEnable(false);
                        mosaic->StartRun();
                        active--;
                    }
                }
            }
            dv_it++;
            // check for chips to enable back
            for(auto si : scan_inf){
                if(!si.second.do_scan[thr_dv]) continue;
                if(dv_it < si.second.dv_range.size() && !chip_confs[si.first]->IsEnabled()){
                    mosaic->StopRun();
                    chips[si.first]->SetEnable(true);
                    mosaic->StartRun();
                    active--;
                }
            }
        }
        // fit threshold curves
        for(int i = 0; i < 1024; i++){
            for(auto si : scan_inf){
                if(!si.second.do_scan[thr_dv]) continue;
                int max     = 0;
                int chip_id = si.first;
                int n_it    = scan_inf[chip_id].n_it[thr_dv]; 
                std::vector<int>  reg_vals = si.second.dv_range;
                std::map<int,int> reg_to_hits;
                // omit "dead" pixels
                for(int reg_val : reg_vals){
                    if(row_hits[chip_id][reg_val][i] > max) max = row_hits[chip_id][reg_val][i];
                    reg_to_hits[reg_val] = row_hits[chip_id][reg_val][i];
                }
                if(max == 0){
                    dead[chip_id].push_back(std::make_tuple(i,row));
                    continue;
                }
                if(!fit_thr_curve(n_it, reg_to_hits, 
                                  fit_funcs[chip_id], 
                                  thr_grs[chip_id], 
                                  thr_hists[chip_id], 
                                  noise_hists[chip_id])) bad[chip_id].push_back(std::make_tuple(i,row));
                int dvmin = *(scan_inf[chip_id].dv_range.begin());
                int dvmax = *(scan_inf[chip_id].dv_range.end() - 1);
                fit_funcs[chip_id]->SetParameters(n_it, (dvmax - dvmin)/2., 0.5);
                fit_funcs[chip_id]->FixParameter(0, n_it);
            }
        }
        if(row%32 == 0) EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Row: " + std::to_string(row) + " DONE");
        // enable chips back
        for(auto si : scan_inf){
            if(!si.second.do_scan[thr_dv]) continue;
            chips[si.first]->SetEnable(true);
        }
    }
    mosaic->StopRun();
    // store scan results
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": writing data into a file");
    std::map<int,int> dv_res;
    TFile* file = new TFile(path.c_str(), "recreate");
    for(auto si : scan_inf){
        if(!si.second.do_scan[thr_dv]) continue;
        thr_hists[si.first]->SetLineColor(kBlue);
        thr_hists[si.first]->SetFillColor(kBlue);
        noise_hists[si.first]->SetLineColor(kBlue);
        noise_hists[si.first]->SetFillColor(kBlue);

        thr_hists[si.first]->Write();
        noise_hists[si.first]->Write();
        thr_grs[si.first]->Write("DebugGraph");

        dv_res[si.first] = std::round(thr_hists[si.first]->GetMean());
    }
    file->Close();
    // enable chips back
    for(auto chip : chips){
        chip.second->SetEnable(true);
    }
    EUDAQ_INFO("Stave_" + std::to_string(this->stave_id) + ": Finished threshold dv scan");
    return dv_res;
}
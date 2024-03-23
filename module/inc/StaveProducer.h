#ifndef StaveProducer_h
#define StaveProducer_h

#include <iostream>
#include <fstream>
#include <algorithm>

#include "eudaq/Producer.hh"

#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"
#include "AlpideConfig.h"

#include "TMath.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"

#include <json/writer.h>
#include <json/reader.h>
#include <json/value.h>

class TAlpide;

//
// TODO: scan interface
//
// scan parameters
typedef enum scan_type{
    digital  = 0,
    fhr      = 1,
    thr_ithr = 2,
    thr_dv   = 3,
} scan_type;

typedef struct{
    // general
    std::map<scan_type,bool> do_scan;
    std::map<scan_type,bool> test_mode;
    std::map<scan_type,int>  n_it;
    
    // threshold scan specific
    std::vector<int> ithr_range;
    std::vector<int> dv_range;
    } scan_info;

class StaveProducer : public eudaq::Producer {
    public:
        StaveProducer(const std::string & name, 
            const std::string & runcontrol);
        void DoInitialise() override;
        void DoConfigure()  override;
        void DoStartRun()   override;
        void DoStopRun()    override;
        void DoTerminate()  override;
        void DoReset()      override;
        void RunLoop()      override;

    static const uint32_t id_factory = eudaq::cstr2hash("StaveProducer");
  
    private:
        // configurations
        TConfig*                            alice_conf;
        TBoardConfigMOSAIC*                 mosaic_conf;
        std::map<std::uint8_t,TChipConfig*> chip_confs;

        // hardware classes
        TReadoutBoardMOSAIC*            mosaic;
        std::map<std::uint8_t,TAlpide*> chips;

        bool running;
        bool configured;

        std::uint8_t stave_id;
        int RCVMAP[9] = {3, 5, 7, 8, 6, 4, 2, 1, 0};
    
        std::string stats_dir;

        // configuration methods
        void setup_mosaic(std::string conf_dir);
        void setup_chips(std::string conf_dir); 
        void setup_chip_conf(eudaq::Configuration c_conf, 
            std::uint8_t chip_id);
        scan_info setup_chip_scans(eudaq::Configuration c_conf, 
            std::uint8_t chip_id);
    
        void readout_stave(int n_chips, 
            std::map<int,std::vector<std::tuple<int,int>>> &return_hits);
    
        // general for scans
        void init_scan(
            std::map<int,scan_info> scan_inf, scan_type st,
            int &max_it, int &row_0, int &max_it_reg, 
            bool &it_global);

        // threshold scans
        void init_thr_analysis(
            std::map<int,scan_info>      scan_inf,
            scan_type                    st,
            std::map<int,TF1*>           &fit_funcs, 
            std::map<int,TGraphErrors*>  &thr_grs, 
            std::map<int,TH1D*>          &thr_hists, 
            std::map<int,TH1D*>          &noise_hists);
        bool fit_thr_curve(
            int n_it,
            std::map<int,int> reg_to_hits, 
            TF1*              fit_func, 
            TGraphErrors*     thr_gr, 
            TH1D*             thr_hist, 
            TH1D*             noise_hist);
        void threshold_scan(
            std::string path, 
            std::map<int,scan_info> scan_inf);
        std::map<int,int> threshold_ithr_scan(
            std::string path, 
            std::map<int,scan_info> scan_inf);
        std::map<int,int> threshold_dv_scan(
            std::string path, 
            std::map<int,scan_info> scan_inf);

        // bad pixels scans
        void fhr_scan(std::string path, 
            std::map<int,scan_info> scan_inf);
        void digital_scan(std::string path, 
            std::map<int,scan_info> scan_inf);
        void config_bad_pixels(
            std::string path, 
            double stuck_tol,
            double fhr_tol);
};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::Producer>::
    Register<StaveProducer, 
        const std::string&, 
        const std::string&>(StaveProducer::id_factory);
}

#endif
#include <iostream>
#include <fstream>

#include "StaveThresholdScanner.hh"

int main() {
    std::string conf_dir = "/home/ssdetlab/stave_alpide/eudaq/user/stave/misc/Tracker.conf";

    auto stave = std::make_shared<StaveController>();

    // get root stave configuration
    auto conf = eudaq::Configuration();

    std::filebuf fb;
    if(!fb.open(conf_dir, std::ios::in)) {
        std::cout << "Error when trying to open configuration file" << std::endl;
        return -1;
    }
    std::istream file(&fb);
    conf.Load(file, "Producer.stave_0"); 
    fb.close();

    stave->stave_id  = conf.Get("STAVE_ID",        0);
    stave->stats_dir = conf.Get("STAVE_STATS_DIR", "");
    // set up ALICE-standardized config file
    std::vector<int> chip_ids;
    for (int i = 0; i < 9; i++) {
        chip_ids.push_back(i);
    }
    stave->alice_conf = new TConfig(1, chip_ids, TBoardType::boardMOSAIC, TDeviceType::TYPE_IBHIC);
    
    // set up MOSAIC board
    std::string mosaic_conf_dir = conf.Get("MOSAIC_CONF_DIR", "");
    stave->setup_mosaic(mosaic_conf_dir);
    
    // set up ALPIDE chips
    std::string chip_conf_dir = conf.Get("ALPIDE_CONF_DIR", "");
    stave->setup_chips(chip_conf_dir);
    
    auto scanner = StaveThresholdScanner(stave);

    int dv_thr            = conf.Get("DV_THR", 30);
    int scan_thr_it       = conf.Get("SCAN_THR_IT", 5);
    int scan_thr_ithr_min = conf.Get("SCAN_THR_ITHR_MIN", 50);
    int scan_thr_ithr_max = conf.Get("SCAN_THR_ITHR_MAX", 90);
    int scan_thr_dv_min   = conf.Get("SCAN_THR_DV_MIN", 10);
    int scan_thr_dv_max   = conf.Get("SCAN_THR_DV_MAX", 30);

    int dv_tol = conf.Get("DV_TOLERANCE", 2);

    std::string stats_dir = conf.Get("STAVE_STATS_DIR", "");

    std::map<int,int> new_ithrs = 
        scanner.ithr_scan(
            stats_dir,
            dv_thr, 
            scan_thr_it,
            scan_thr_ithr_min,
            scan_thr_ithr_max);

    // apply new ITHR values
    for (auto& [id,ithr] : new_ithrs) {
        auto chip = (*stave->chips.find(id)).second;
        stave->mosaic->WriteChipRegister(
            Alpide::REG_ITHR, ithr, chip);
    }

    // validate the DV
    std::map<int,int> dvs = 
        scanner.dv_scan(
            stats_dir,
            scan_thr_it,
            scan_thr_dv_min,
            scan_thr_dv_max);

    // Save validated data
    for (auto& [id,dv] : dvs) {
        if (abs(dv - dv_thr) <= dv_tol) {
            std::cout 
            << "Threshold scan for Chip " 
            << id 
            << " succeeded:" 
            << " Expected DV = " << dv_thr 
            << ", Got DV = " << dv << std::endl;

            // set up chip configuration directory
            std::string chip_conf_name = "ALPIDE_" + std::to_string(id) + ".conf";
        
            // load configuration file 
            auto c_conf = eudaq::Configuration();
            std::fstream file_in(chip_conf_dir + chip_conf_name, std::ios::in);
            c_conf.Load(file_in, "ALPIDE"); 
            file_in.close();

            c_conf.Set("ITHR", new_ithrs.at(id));
            c_conf.Set("SCAN_THR_DV",   0);
            c_conf.Set("SCAN_THR_ITHR", 0);

            std::fstream file_out(chip_conf_dir + chip_conf_name, std::ios::out);
            c_conf.Save(file_out);
            file_out.close();
        } else {
            std::cout 
            << "Threshold scan for Chip " 
            << id 
            << " failed:" 
            << " Expected DV = " << dv_thr 
            << ", Got DV = " << dv << std::endl;
        } 
    }


    return 0;
}
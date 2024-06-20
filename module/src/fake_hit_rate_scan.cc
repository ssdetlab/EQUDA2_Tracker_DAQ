#include <iostream>
#include <fstream>

#include "StaveFakeHitRateScanner.hh"

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
    
    auto scanner = StaveFakeHitRateScanner(stave);

    int n_it              = conf.Get("SCAN_FHR_IT", 30);
    std::string stats_dir = conf.Get("STAVE_STATS_DIR", "");

    scanner.fhr_scan(stats_dir, n_it);

    return 0;
}
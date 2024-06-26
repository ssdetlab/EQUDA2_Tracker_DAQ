#include "StaveProducer.h"
#include <fstream>
#include <unordered_map>

#include "json/writer.h"
#include "json/reader.h"
#include "json/value.h"

void StaveController::setup_mosaic(std::string conf_dir) {
    // set up configuration's name
    std::string mosaic_conf_name = 
        "Mosaic_" + std::to_string(this->stave_id) + ".conf";
    conf_dir.append(mosaic_conf_name);
    
    // load configuration file 
    auto m_conf = eudaq::Configuration();
    std::filebuf fb;
    if(!fb.open(conf_dir, std::ios::in)) {
        throw std::runtime_error(
            "Error when trying to open configuration file for MOSAIC_" 
            + std::to_string(this->stave_id) + ". Aborting producer.");
        return;
    }
    std::istream file(&fb);
    m_conf.Load(file, "MOSAIC"); 

    // set up MOSAIC configuration class
    std::string m_ip_addr  = m_conf.Get("IP_ADDRESS",           "192.168.168.250");
    int  m_tcp_addr        = m_conf.Get("TCP_PORT_ADDR",        2000);
    int  m_nof_ctrl_ifc    = m_conf.Get("NOF_CTRL_IFC",         1);
    int  m_ctrl_ifc_ph     = m_conf.Get("CTRL_IFC_PHASE",       2);
    int  m_ctrl_af_thr     = m_conf.Get("CTRL_AF_THR",          1250000);
    int  m_ctrl_lat_mode   = m_conf.Get("CTRL_LATENCY_MODE",    0);
    int  m_ctrl_timeout    = m_conf.Get("CTRL_TIMEOUT",         0);
    int  m_data_link_pol   = m_conf.Get("DATA_LINK_POLARITY",   0);
    int  m_trig_del        = m_conf.Get("TRIG_DELAY",           20);
    int  m_pulse_del       = m_conf.Get("PULSE_DELAY",          10000);
    int  m_poll_timeout    = m_conf.Get("POLLING_DATA_TIMEOUT", 500);
    auto m_data_link_speed = static_cast<Mosaic::TReceiverSpeed>(m_conf.Get("DATA_LINK_SPEED", 0));
    int  m_manchester_dis  = m_conf.Get("MANCHESTER_DISABLED",  0);

    mosaic_conf = new TBoardConfigMOSAIC();
    mosaic_conf->SetIPaddress(m_ip_addr.c_str());
    mosaic_conf->SetTCPport(m_tcp_addr);
    mosaic_conf->SetCtrlInterfaceNum(m_nof_ctrl_ifc);
    mosaic_conf->SetCtrlInterfacePhase(m_ctrl_ifc_ph);
    mosaic_conf->SetCtrlAFThreshold(m_ctrl_af_thr);
    mosaic_conf->SetCtrlLatMode(m_ctrl_lat_mode);
    mosaic_conf->SetCtrlTimeout(m_ctrl_timeout);
    mosaic_conf->SetInvertedData(m_data_link_pol);
    mosaic_conf->SetSpeedMode(m_data_link_speed);
    mosaic_conf->SetTriggerDelay(m_trig_del);
    mosaic_conf->SetPulseDelay(m_pulse_del);  
    mosaic_conf->SetPollingDataTimeout(m_poll_timeout);
    mosaic_conf->SetManchesterDisable(m_manchester_dis);
    
    mosaic = new TReadoutBoardMOSAIC(alice_conf, mosaic_conf);
}

void StaveController::setup_chip_conf(eudaq::Configuration c_conf, std::uint8_t chip_id) {
    // set up chip and configuration classes
    TChipConfig* chip_conf = new TChipConfig(alice_conf, chip_id);
    
    // general
    chip_conf->SetParamValue("CHIP_ID",          chip_id);
    chip_conf->SetParamValue("ENABLED",          c_conf.Get("ENABLE",   1));
    chip_conf->SetParamValue("ENABLEDBB",        c_conf.Get("ENABLEBB", 1));
    chip_conf->SetParamValue("CONTROLINTERFACE", c_conf.Get("CTRL_IFC", 0));
    chip_conf->SetParamValue("RECEIVER",         c_conf.Get("RECEIVER", RCVMAP[chip_id]));
    
    // DAC configuration
    chip_conf->SetParamValue("VRESETD", c_conf.Get("VRESETD", ChipConfig::VRESETD));
    chip_conf->SetParamValue("VCASN",   c_conf.Get("VCASN",   ChipConfig::VCASN));
    chip_conf->SetParamValue("VCASN2",  c_conf.Get("VCASN2",  ChipConfig::VCASN2));
    chip_conf->SetParamValue("VCLIP",   c_conf.Get("VCLIP",   ChipConfig::VCLIP));
    chip_conf->SetParamValue("ITHR",    c_conf.Get("ITHR",    ChipConfig::ITHR));
    chip_conf->SetParamValue("VPULSEH", c_conf.Get("VPULSEH", ChipConfig::VPULSEH));
    chip_conf->SetParamValue("VPULSEL", c_conf.Get("VPULSEL", ChipConfig::VPULSEL));
    
    chip_conf->SetParamValue("IDB",     c_conf.Get("IDB",     ChipConfig::IDB));
    chip_conf->SetParamValue("IBIAS",   c_conf.Get("IBIAS",   ChipConfig::IBIAS));
    chip_conf->SetParamValue("VCASP",   c_conf.Get("VCASP",   ChipConfig::VCASP));
    chip_conf->SetParamValue("VRESETP", c_conf.Get("VRESETP", ChipConfig::VRESETP));
    chip_conf->SetParamValue("VTEMP",   c_conf.Get("VTEMP",   ChipConfig::VTEMP));
    chip_conf->SetParamValue("IAUX2",   c_conf.Get("IAUX2",   ChipConfig::IAUX2));
    chip_conf->SetParamValue("IRESET",  c_conf.Get("IRESET",  ChipConfig::IRESET));
    
    // mode control configuration
    chip_conf->SetParamValue("READOUTMODE",        c_conf.Get("RO_MODE",           ChipConfig::READOUT_MODE));
    chip_conf->SetParamValue("ENABLECLUSTERING",   c_conf.Get("EN_CLUSTERING",     ChipConfig::ENABLE_CLUSTERING));
    chip_conf->SetParamValue("MATRIXREADOUTSPEED", c_conf.Get("MAT_RO_SPEED",      ChipConfig::MATRIX_READOUT_SPEED));
    chip_conf->SetParamValue("ENABLESKEWGLOBAL",   c_conf.Get("EN_GLB_SKEW",       ChipConfig::ENABLE_SKEWING_GLOBAL));
    chip_conf->SetParamValue("ENABLESKEWSTARTRO",  c_conf.Get("EN_RO_SKEW",        ChipConfig::ENABLE_SKEWING_STARTRO));
    chip_conf->SetParamValue("ENABLECLKGATE",      c_conf.Get("EN_CLK_GATE",       ChipConfig::ENABLE_CLOCK_GATING));
    chip_conf->SetParamValue("ENABLECMURO",        c_conf.Get("EN_CMU_RO",         ChipConfig::ENABLE_CMU_READOUT));
    chip_conf->SetParamValue("SERIALLINKSPEED",    c_conf.Get("SERIAL_LINK_SPEED", ChipConfig::SERIAL_LINK_SPEED));
    
    // FROMU configuration register 1
    chip_conf->SetParamValue("MEBMASK",            c_conf.Get("MEB_MASK",              ChipConfig::MEB_MASK));
    chip_conf->SetParamValue("INTSTROBEGEN",       c_conf.Get("INT_STROBE_GEN",        ChipConfig::INT_STROBE_GEN));
    chip_conf->SetParamValue("BUSYMON",            c_conf.Get("EN_BUSY_MON",           ChipConfig::BUSY_MON));
    chip_conf->SetParamValue("TESTPULSEMODE",      c_conf.Get("TEST_PULSE_MODE",       ChipConfig::TEST_PULSE_MODE));
    chip_conf->SetParamValue("ENTESTSTROBE",       c_conf.Get("EN_TEST_STROBE",        ChipConfig::EN_TEST_STROBE));
    chip_conf->SetParamValue("ENROTATEPULSELINES", c_conf.Get("EN_ROTATE_PULSE_LINES", ChipConfig::EN_ROTATE_PULSE_LINES));
    chip_conf->SetParamValue("TRIGGERDELAY",       c_conf.Get("PULSE_TRG_DELAY",       ChipConfig::TRIGGER_DELAY));
    
    // FROMU configuration register 2
    chip_conf->SetParamValue("STROBEDURATION",  c_conf.Get("STROBE_DUR", ChipConfig::STROBE_DURATION));
    
    // FROMU configuration register 3
    chip_conf->SetParamValue("STROBEGAP", c_conf.Get("STROBE_GAP", ChipConfig::STROBE_GAP));
    
    // FROMU pulsing register 1
    chip_conf->SetParamValue("STROBEDELAYCHIP", c_conf.Get("STROBE_DELAY", ChipConfig::STROBE_DELAY));
    
    // FROMU pulsing register 2
    chip_conf->SetParamValue("PULSEDURATION",   c_conf.Get("PULSE_DUR",    ChipConfig::PULSE_DURATION));
    
    // CMU/DMU configuration
    chip_conf->SetParamValue("PREVID",             c_conf.Get("PREV_ID",        -1));
    chip_conf->SetParamValue("INITIALTOKEN",       c_conf.Get("INIT_TOKEN",     0));
    chip_conf->SetParamValue("MANCHESTERDISABLED", c_conf.Get("DIS_MANCHESTER", ChipConfig::DISABLE_MANCHESTER));
    chip_conf->SetParamValue("ENABLEDDR",          c_conf.Get("EN_DDR",         ChipConfig::ENABLE_DDR));
    
    // misc
    chip_conf->SetParamValue("PLLPHASE",    c_conf.Get("PLL_PHASE",    ChipConfig::PLL_PHASE));
    chip_conf->SetParamValue("PLLSTAGES",   c_conf.Get("PLL_STAGES",   ChipConfig::PLL_STAGES));
    chip_conf->SetParamValue("CHARGEPUMP",  c_conf.Get("CHARGE_PUMP",  ChipConfig::CHARGE_PUMP));
    chip_conf->SetParamValue("DTUDRIVER",   c_conf.Get("DTU_DRIVER",   ChipConfig::DTU_DRIVER));
    chip_conf->SetParamValue("DTUPREEMP",   c_conf.Get("DTU_PREEMP",   ChipConfig::DTU_PREEMP));
    chip_conf->SetParamValue("DCTRLDRIVER", c_conf.Get("DCTRL_DRIVER", ChipConfig::DCTRL_DRIVER));

    // Scans
    chip_conf->SetParamValue("SCAN_THR_ITHR", c_conf.Get("SCAN_THR_ITHR", ChipConfig::SCAN_THR_ITHR));
    chip_conf->SetParamValue("SCAN_THR_DV",   c_conf.Get("SCAN_THR_DV",   ChipConfig::SCAN_THR_DV));
    chip_conf->SetParamValue("SCAN_FHR",      c_conf.Get("SCAN_FHR",      ChipConfig::SCAN_FHR));

    // store chip configuration in 
    // the internal class buffers 
    chip_confs[chip_id] = chip_conf;
}

void StaveController::config_bad_pixels(std::string path) {
    std::fstream fin;
    path += "stave_" + std::to_string(stave_id) + ".json";
    fin.open(path, std::ios::in);
    Json::Value root;
    Json::Reader reader;
    reader.parse(fin, root);
    fin.close();
    for (auto id : root.getMemberNames()) {
        std::vector<int> xs;
        std::vector<int> ys;
        for (auto ix : root[id]["bad_pixels"]["pix_x"]) {
            int pix_x = ix.asInt();
            xs.push_back(pix_x);
        }
        for (auto iy : root[id]["bad_pixels"]["pix_y"]) {
            int pix_y = iy.asInt();
            ys.push_back(pix_y);
        }
        for (int i = 0; i < xs.size(); i++) {
            AlpideConfig::WritePixRegSingle(
                chips.at(static_cast<std::uint8_t>(std::stoi(id))), 
                Alpide::PIXREG_MASK, 
                true, 
                ys.at(i), 
                xs.at(i));
        }
    }
}

void StaveController::setup_chips(std::string conf_dir) {
    // set up chip configurations
    for (int i = 0; i < 9; i++) {
        // set up chip configuration directory
        std::string chip_conf_name = "ALPIDE_" + std::to_string(i) + ".conf";
    
        // load configuration file 
        auto c_conf = eudaq::Configuration();
        std::filebuf fb;
        if(!fb.open(conf_dir + chip_conf_name, std::ios::in)) {
            throw std::runtime_error(
                "Error when trying to open configuration file for chip " 
                + std::to_string(i) + ".");
        }
        std::istream file(&fb);
        c_conf.Load(file, "ALPIDE"); 
    
        setup_chip_conf(c_conf, i);
    
        // initialize chip class and connect it 
        // to MOSAIC
        if (!chip_confs[i]->IsEnabled()) {
            continue;
        }
        TAlpide* chip = new TAlpide(chip_confs[i]);
        chips[i] = chip;
        chip->SetReadoutBoard(dynamic_cast<TReadoutBoard*>(this->mosaic));
        mosaic->AddChip(i, 0, RCVMAP[i], chip);
    }

    // apply chip configuration
    for (auto [id,chip] : chips) {
        mosaic->SendOpCode(Alpide::OPCODE_GRST, chip);
        mosaic->SendOpCode(Alpide::OPCODE_PRST, chip);
        AlpideConfig::ConfigureDACs(chip);
        AlpideConfig::ConfigureCMUDMU(chip);
        AlpideConfig::ConfigureFromu(chip);
        AlpideConfig::ConfigurePLL(chip);
        AlpideConfig::ConfigureModeControl(chip);
        AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, false);
        AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);
        mosaic->SendOpCode(Alpide::OPCODE_RORST, chip);
    }
}

StaveProducer::StaveProducer(
    const std::string & name, 
    const std::string & runcontrol)
        : eudaq::Producer(name, runcontrol), 
        running(false), configured(false) {}

void StaveProducer::DoInitialise() {
    control = std::make_shared<StaveController>();
}

void StaveProducer::DoConfigure() {
    // in case it's a second CONFIG call
    if (configured) {
        DoReset();
    } 
    
    // get root stave configuration
    auto conf = GetConfiguration();
    
    control->stave_id  = conf->Get("STAVE_ID",        0);
    control->stats_dir = conf->Get("STAVE_STATS_DIR", "");
    // set up ALICE-standardized config file
    std::vector<int> chip_ids;
    for (int i = 0; i < 9; i++) {
        chip_ids.push_back(i);
    }
    control->alice_conf = new TConfig(1, chip_ids, TBoardType::boardMOSAIC, TDeviceType::TYPE_IBHIC);
    
    // set up MOSAIC board
    std::string mosaic_conf_dir = conf->Get("MOSAIC_CONF_DIR", "");
    control->setup_mosaic(mosaic_conf_dir);
    
    // set up ALPIDE chips
    std::string chip_conf_dir = conf->Get("ALPIDE_CONF_DIR", "");
    control->setup_chips(chip_conf_dir);
    
    // configure bad pixels
    // control->config_bad_pixels(control->stats_dir);
    
    // configure MOSAIC triggers
    control->mosaic->SetTriggerConfig(
        false, false, 
        control->mosaic->GetConfig()->GetTriggerDelay(), 
        control->mosaic->GetConfig()->GetPulseDelay());
    control->mosaic->SetTriggerSource(trigExt);

    configured = true;
}

void StaveProducer::RunLoop() {
    // initialize counters
    std::uint16_t non_empty_frames = 0, trg_n = 0;
    
    control->mosaic->SendOpCode(Alpide::OPCODE_BCRST);
    control->mosaic->StartRun();
    while (running) {
        // poll all of the active chips
        std::vector<std::uint8_t> block;
        std::vector<int> n_bytes_data;
        for (int i = 0; i < control->chips.size(); i++) {
            std::uint8_t buffer[MAX_EVENT_SIZE];
            int data_size_dummy;
            int res = control->mosaic->ReadEventData(data_size_dummy, buffer);
            // discard no data events
            if (res == 0 || (buffer[64] & 0xf0) == 0xe0) {
                continue;
            }
            n_bytes_data.push_back(data_size_dummy);
            // std::cout << "DATA SIZE = " << data_size_dummy << std::endl;
            block.insert(block.end(), std::begin(buffer), std::end(buffer));
        }
        // if no data were found 
        // skip event
        if(block.size() == 0) {
            continue;
        }
        auto ev = eudaq::Event::MakeUnique("StaveRaw");
        ev->SetTag("Stave ID", std::to_string(control->stave_id));
        ev->AddBlock(control->stave_id, block);
        for (int i = 0; i < block.size()/MAX_EVENT_SIZE; i++) {
            ev->SetTag("Event size " + std::to_string(i), n_bytes_data[i]);
        }
        // This is a dummy trigger ID
        // The true one comes from the TLU
        ev->SetTriggerN(trg_n);
        non_empty_frames++;
        trg_n++;
        SendEvent(std::move(ev)); 
    }
    control->mosaic->StopRun();

    EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + ": MOSAIC triggers sent:" + std::to_string(control->mosaic->GetTriggerCount()));
    for (auto chip : control->chips) {
        uint16_t val;
        control->mosaic->ReadChipRegister(0x0009, val, chip.second);
        EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + " ALPIDE_ " 
                            + std::to_string(chip.second->GetConfig()->GetChipId()) + ": Triggers:" 
                            + std::to_string(val));
        control->mosaic->ReadChipRegister(0x000A, val, chip.second);
        EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + " ALPIDE_ " 
                            + std::to_string(chip.second->GetConfig()->GetChipId()) + ": Strobes:"         
                            + std::to_string(val));
        control->mosaic->ReadChipRegister(0x000B, val, chip.second);
        EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + " ALPIDE_ " 
                            + std::to_string(chip.second->GetConfig()->GetChipId()) + ": Matrix readouts:" 
                            + std::to_string(val));
        control->mosaic->ReadChipRegister(0x000C, val, chip.second);
        EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + " ALPIDE_ " 
                            + std::to_string(chip.second->GetConfig()->GetChipId()) + ": Frames:" 
                            + std::to_string(val));
    }
    EUDAQ_INFO("Stave:" + std::to_string(control->stave_id) + " Non-Empty Frames:" + std::to_string(non_empty_frames));
}

void StaveProducer::DoStartRun() {
    running = true;
}

void StaveProducer::DoStopRun() {
    running = false;
}

void StaveProducer::DoReset() {
    running = false;
}

void StaveProducer::DoTerminate() {
    running = false;
}
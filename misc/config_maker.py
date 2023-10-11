import sys
import configparser
import argparse

# root terminal input parser
arg_parse = argparse.ArgumentParser() 

def create_mosaic_conf(root_dir, stave_id):
    conf = configparser.ConfigParser()
    conf.optionxform = str
    conf.add_section('MOSAIC')

    # IP address
    conf.set('MOSAIC', 'IP_ADDRESS', '192.168.168.' + str(250 + stave_id))

    # TCP/IP port address 
    conf.set('MOSAIC', 'TCP_PORT_ADDR', str(2000 + stave_id))

    # delay between strobe and next pulse
    conf.set('MOSAIC', 'PULSE_DELAY', '10000')

    conf.set('MOSAIC', 'TRIG_DELAY', '0')

    # number of active control interfaces
    conf.set('MOSAIC', 'NOF_CTRL_IFC', '12')

    # phase of Control Interfaces phase. default value 2
    conf.set('MOSAIC', 'CTRL_IFC_PHASE', '2')

    # almost full threshold 
    # of MOSAIC internal buffers
    conf.set('MOSAIC', 'CTRL_AF_THR', '1250000')

    # latency mode for the TCP/IP packet transmission. 
    # 0 = End Of Event, 1 = Memory, 2 = Timeout
    conf.set('MOSAIC', 'CTRL_LATENCY_MODE', '0')

    # timeout in the TCP/IP transmission. Default 0
    conf.set('MOSAIC', 'CTRL_TIMEOUT', '0')

    # timeout in the ReadData polling cycle in milliseconds
    conf.set('MOSAIC', 'POLLING_DATA_TIMEOUT', '500')

    # enable or disable the polarity inversion 
    # of HSDL lines. 0 = Not inverted
    conf.set('MOSAIC', 'DATA_LINK_POLARITY', '0')

    # HSDL speed definition. 
    # 0 = 400 MHz, 1 = 600 MHz, 2 = 1.2GHz
    conf.set('MOSAIC', 'DATA_LINK_SPEED', '0')

    # enable or disable Manchester Encoding of Control 
    # Interfaces. 0 = Enabled, 1 = Disabled
    conf.set('MOSAIC', 'MANCHESTER_DISABLED', '0')

    # for the synchronised triggering/pulsing 
    # of two MOSAIC boards, 
    # default: 0 = disabled
    conf.set('MOSAIC', 'MASTER_SLAVE', '0')

    with open(root_dir + '/mosaic_confs/' + '/Mosaic_' + str(stave_id) + '.conf', "w") as configfile:
        conf.write(configfile)


def create_chip_conf(root_dir, stave_id, chip_id):
    conf = configparser.ConfigParser()
    conf.optionxform = str
    conf.add_section('ALPIDE')

    # general
    conf.set('ALPIDE', 'ENABLE',   '1')
    conf.set('ALPIDE', 'ENABLEBB', '1')
    conf.set('ALPIDE', 'CTRL_IFC', '0')

    # CMU/DMU configuration
    conf.set('ALPIDE', 'DIS_MANCHESTER', '0x0')
    conf.set('ALPIDE', 'EN_DDR',         '0x1')

    # scan flags
    conf.set('ALPIDE', 'SCAN_DIGITAL',      '0')
    conf.set('ALPIDE', 'SCAN_DIGITAL_TEST', '0')
    conf.set('ALPIDE', 'SCAN_DIGITAL_IT',   '10')

    conf.set('ALPIDE', 'SCAN_FHR',      '0')
    conf.set('ALPIDE', 'SCAN_FHR_TEST', '0')
    conf.set('ALPIDE', 'SCAN_FHR_IT',   '1000')

    # FROMU configuration register 1
    conf.set('ALPIDE', 'MEB_MASK',              '0x0')
    conf.set('ALPIDE', 'INT_STROBE_GEN',        '0x0')
    conf.set('ALPIDE', 'EN_BUSY_MON',           '1x0')
    conf.set('ALPIDE', 'TEST_PULSE_MODE',       '0x0')
    conf.set('ALPIDE', 'EN_TEST_STROBE',        '0x0')
    conf.set('ALPIDE', 'EN_ROTATE_PULSE_LINES', '0x0')
    conf.set('ALPIDE', 'PULSE_TRG_DELAY',       '0')

    # FROMU configuration register 2
    conf.set('ALPIDE', 'STROBE_DUR', '38000')

    # FROMU configuration register 3
    conf.set('ALPIDE', 'STROBE_GAP', '0')

    # FROMU pulsing register 1
    conf.set('ALPIDE', 'STROBE_DELAY', '0')

    # FROMU pulsing register 2
    conf.set('ALPIDE', 'PULSE_DUR', '500')

    # chip mode control configuration
    conf.set('ALPIDE', 'RO_MODE',           '0x1')
    conf.set('ALPIDE', 'EN_CLUSTERING',     '0x1')
    conf.set('ALPIDE', 'MAT_RO_SPEED',      '0x1')
    conf.set('ALPIDE', 'SERIAL_LINK_SPEED', '400')
    conf.set('ALPIDE', 'EN_GLB_SKEW',       '0x0')
    conf.set('ALPIDE', 'EN_RO_SKEW',        '0x0')
    conf.set('ALPIDE', 'EN_CLK_GATE',       '0x0')
    conf.set('ALPIDE', 'EN_CMU_RO',         '0x0')

    # chip readout ports configuration
    conf.set('ALPIDE', 'EN_DCTRL_RO',    '0')
    conf.set('ALPIDE', 'DCTRL_RO_DELAY', '1000')

    # DAC configuration
    conf.set('ALPIDE', 'ITHR',    '95')
    conf.set('ALPIDE', 'IDB',     '29')
    conf.set('ALPIDE', 'VCASN',   '50')
    conf.set('ALPIDE', 'VCASN2',  '57')
    conf.set('ALPIDE', 'VCLIP',   '0')
    conf.set('ALPIDE', 'VRESETD', '147')
    conf.set('ALPIDE', 'VCASP',   '86')
    conf.set('ALPIDE', 'IBIAS',   '64')
    conf.set('ALPIDE', 'VPULSEH', '170')
    conf.set('ALPIDE', 'VPULSEL', '140')
    conf.set('ALPIDE', 'VRESETP', '117')
    conf.set('ALPIDE', 'VTEMP',   '0')
    conf.set('ALPIDE', 'IAUX2',   '0')
    conf.set('ALPIDE', 'IRESET',  '50')

    # threshold scan flags
    # values of VCASN, VCASN2,
    # VRESETD, VCLIP, VPULSEH and 
    # SPULSEL will be taken 
    # as specified above 
    conf.set('ALPIDE', 'SCAN_THR',          '1')
    conf.set('ALPIDE', 'SCAN_THR_TEST',     '0')
    conf.set('ALPIDE', 'SCAN_THR_IT',       '50')
    conf.set('ALPIDE', 'SCAN_THR_ITHR_MIN', '0')
    conf.set('ALPIDE', 'SCAN_THR_ITHR_MAX', '120')
    conf.set('ALPIDE', 'SCAN_THR_DV_MIN',   '0')
    conf.set('ALPIDE', 'SCAN_THR_DV_MAX',   '50')

    with open(root_dir + '/alpide_confs/stave_' + str(stave_id) + '/ALPIDE_' + str(chip_id) + '.conf', "w") as configfile:
        conf.write(configfile)

def setup_conf(root_dir, n_staves):
    conf = configparser.ConfigParser()
    conf.optionxform = str
    # RunControl configuration
    conf.add_section('RunControl')
    # Steer which values to display in the GUI: producerName and displayed value are seperated by a ",". 
    conf.set('RunControl', 'ADDITIONAL_DISPLAY_NUMBERS', "log,_SERVER")

    # Stave Producers configuration
    for i in range(n_staves):
        sect = 'Producer.stave_' + str(i)
        conf.add_section(sect)
        # directories to search MOSAIC and 
        # ALPIDE configurations in
        conf.set(sect, 'MOSAIC_CONF_DIR', root_dir + '/mosaic_confs/')
        conf.set(sect, 'ALPIDE_CONF_DIR', root_dir + '/alpide_confs/stave_' + str(i) + '/')
        # path to the stave stats
        conf.set(sect, 'STAVE_STATS_DIR', root_dir + '/stave_stats/stave_' + str(i) + '/')
        # stave identificator
        conf.set(sect, 'STAVE_ID', str(i))
        # bad pixel tolerance
        conf.set(sect, 'DIGITAL_STUCK_TOL', '0')
        conf.set(sect, 'FHR_TOL',           '0')
        # connection to the data collector
        conf.set(sect, 'EUDAQ_DC', 'my_dc')
        # setup chips
        for j in range(9):
            create_chip_conf(root_dir, i, j)
        # setup mosaic
        create_mosaic_conf(root_dir, i)

    # Data Collector configuration
    conf.add_section('DataCollector.my_dc')
    # connection to the monitor
    conf.set('DataCollector.my_dc', 'EUDAQ_MN', 'my_mon')
    conf.set('DataCollector.my_dc', 'EUDAQ_FW', 'native')
    conf.set('DataCollector.my_dc', 'EUDAQ_FW_PATTERN', root_dir + '/ev_data/$12D_run$6R$X')
    conf.set('DataCollector.my_dc', 'EUDAQ_DATACOL_SEND_MONITOR_FRACTION', '1')
    # data storage
    conf.set('DataCollector.my_dc', 'TTREE_DATA_PATH', root_dir + '/ev_data/ttree_data/tree.root')
    conf.set('DataCollector.my_dc', 'TTREE_NAME',      "MyTree")
    conf.set('DataCollector.my_dc', 'BUF_SIZE',        '32000')
    conf.set('DataCollector.my_dc', 'SPLIT_LVL',       '0')
    # monitoring and sync
    conf.set('DataCollector.my_dc', 'ONLINE_MONITOR', '0')
    conf.set('DataCollector.my_dc', 'NOF_STAVES',     '1')
    conf.set('DataCollector.my_dc', 'MIN_STAVES',     '0')
    conf.set('DataCollector.my_dc', 'MIN_EV_TO_DUMP', '10')

    # Monitor configuration
    conf.add_section('Monitor.my_mon')
    conf.set('Monitor.my_mon', 'NOF_STAVES',  str(n_staves))
    for i in range(n_staves):
        conf.set('Monitor.my_mon', 'NOF_CHIPS_' + str(i), '9')

    with open(root_dir + '/Tracker.conf', "w") as configfile:
        conf.write(configfile)

def setup_ini(root_dir):
    conf = configparser.ConfigParser()
    conf.optionxform = str

    conf.add_section('logCollector.log')
    # path to store all the log files
    conf.set('logCollector.log', 'EULOG_GUI_LOG_FILE_PATTERN', root_dir + '/log/tracker_$12D.log')

    with open(root_dir + '/Tracker.ini', "w") as inifile:
        conf.write(inifile)

def main():
    args = sys.argv[1:]
    n_staves = 0
    root_dir = ""
    if args[0] == '--nof_staves':
        n_staves = args[1]
    if args[2] == '--root_dir':
        root_dir = args[3]
    setup_conf(root_dir, int(n_staves))

main()
#!/bin/bash

# set current directory
current_dir=$(pwd)
echo "Set the current number of staves:"
read n_staves

# create directory structure for the configuration files
mkdir -p $current_dir/ev_data/ttree_data
mkdir -p $current_dir/log
for (( i = 0; i < $n_staves; i++ )) do
    mkdir -p $current_dir/alpide_confs/stave_$i
    mkdir -p $current_dir/mosaic_confs
    mkdir -p $current_dir/stave_stats/stave_$i/thr_scans    

    touch $current_dir/mosaic_confs/Mosaic_$i.conf
    for (( j = 0; j < 9; j++ )) do
        touch $current_dir/alpide_confs/stave_$i/ALPIDE_$j.conf
    done
done
touch $current_dir/Tracker.conf
touch $current_dir/Tracker.ini

# setup configuration files
python3.10 $current_dir/config_maker.py --nof_staves $n_staves --root_dir $current_dir
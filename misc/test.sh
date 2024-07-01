#!/usr/bin/env sh
BINPATH=../../../bin
$BINPATH/euRun -a tcp://45368 &
sleep 1
$BINPATH/euLog -r tcp://192.168.168.1:45368 -a tcp://45370 &
sleep 1

$BINPATH/euCliCollector -n DummyDataCollector -t my_dc -r tcp://192.168.168.1:45368 &
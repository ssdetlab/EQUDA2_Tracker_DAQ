#!/usr/bin/env sh
BINPATH=../../../bin
$BINPATH/euRun -a tcp://45368 &
sleep 1
$BINPATH/euLog -r tcp://localhost:45368 -a tcp://45370 &
sleep 1
$BINPATH/euCliMonitor -n StaveROOTMonitor -t my_mon -r tcp://localhost:45368 -a tcp://44375 &

$BINPATH/euCliCollector -n StaveTTreeDataCollector -t my_dc -r tcp://localhost:45368 -a tcp://45373 &
# $BINPATH/euCliCollector -n StaveTTreeDataCollectorTLUSync -t my_dc -r tcp://localhost:45368 -a tcp://45373 &

$BINPATH/euCliProducer -n StaveProducer -t stave_0 -r tcp://localhost:45368 &
# $BINPATH/euCliProducer -n AidaTluProducer -t aida_tlu -r tcp://localhost:45368 &
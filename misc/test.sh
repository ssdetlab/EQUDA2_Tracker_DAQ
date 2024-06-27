#!/usr/bin/env sh
BINPATH=../../../bin
$BINPATH/euRun -a tcp://45368 &
sleep 1
$BINPATH/euLog -r tcp://localhost:45368 -a tcp://45370 &
sleep 1

$BINPATH/euCliProducer -n DummyProducer -t dummy -r tcp://localhost:45368 &
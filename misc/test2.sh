#!/usr/bin/env sh
BINPATH=../../../bin

$BINPATH/euCliProducer -n DummyProducer -t dummy -r tcp://localhost:45368 &
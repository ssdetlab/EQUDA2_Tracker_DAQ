#!/bin/bash

# voltages to be used with the 9m cables
VOLTAGES=(1.930 2.090 1.970 2.130 1.980 2.140 1.980 2.150 1.990 2.160 2.010 2.110 2.050 2.210 1.895 1.895 )
IP=192.168.168.250

HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PBCTRL=${HOME_DIR}/pbctrl

# get to a defined state
${PBCTRL} -off ${IP} ;
${PBCTRL} -state ${IP}
sleep 1

# set voltages
for ch in $(seq 0 13)
do
	${PBCTRL} -Vout $ch ${VOLTAGES[ch]} ${IP}
done
sleep 1

# analogue
for ch in $(seq 0 2 12)
do
    ${PBCTRL} -Ith $ch 3.0 ${IP}
    ${PBCTRL} -onch $ch ${IP}
    ${PBCTRL} -Ith $ch 0.5 ${IP}
done

sleep 1 # wait

# digital
for ch in $(seq 1 2 13)
do
    ${PBCTRL} -Ith $ch 3.0 ${IP}
    ${PBCTRL} -onch $ch ${IP}
    ${PBCTRL} -Ith $ch 1.5 ${IP}
done

sleep 1 # wait

${PBCTRL} -state ${IP}
echo "### Stave ON ###"

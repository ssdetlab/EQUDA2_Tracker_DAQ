#!/bin/bash

HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PBCTRL=${HOME_DIR}/pbctrl

${PBCTRL} -off 192.168.168.250 ;
${PBCTRL} -state 192.168.168.250

echo "### Stave OFF ###"

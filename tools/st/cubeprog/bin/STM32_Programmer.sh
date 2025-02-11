#!/usr/bin/env bash
MYROOTDIR=`dirname ${0}`
export LD_LIBRARY_PATH=${MYROOTDIR}/../lib:${LD_LIBRARY_PATH}
${MYROOTDIR}/STM32_Programmer_CLI "$@"


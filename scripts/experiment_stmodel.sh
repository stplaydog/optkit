#!/bin/bash

#
# Copy right YMSys, 2015,2016 Zhaoming Yin
#
# @brief    This script runs experiment for stmodel 
#
#   MODIFIED   (MM/DD/YY)
#   stplaydog   09/17/16 - Add options to output cohesive subgraphs or not. 
#   stplaydog   08/28/16 - Run only on core data 
#   stplaydog   08/20/16 - Avoid redundant computation 
#   stplaydog   08/12/16 - Also copy log for time analysis 
#   stplaydog   08/06/16 - Parallel computing 
#   stplaydog   04/27/16 - Add upload result script 
#   stplaydog   04/24/16 - Add logger support 
#   stplaydog   04/16/16 - Add real data support 
#   stplaydog   03/05/16 - Creation
#

SCRIPT_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OPTKIT_HOME=${SCRIPT_HOME}/../src/
DATA_HOME=${SCRIPT_HOME}/../data/
LOG_HOME=${SCRIPT_HOME}/../log/

mkdir -p ${DATA_HOME}/experiments/stmodel/
mkdir -p ${DATA_HOME}/experiments/stmodel/CHI
mkdir -p ${DATA_HOME}/experiments/stmodel/DC
mkdir -p ${DATA_HOME}/experiments/stmodel/NY

rm ${SCRIPT_HOME}/process
rm ${LOG_HOME}/*
touch ${SCRIPT_HOME}/process

NUM_PROC=`nproc`

BINARY_INFO=`objdump -h ${OPTKIT_HOME}/optkit | grep debug | wc -l`
if [[ ${BINARY_INFO} != *"0"* ]];
then
    echo "You need to use the release build to run the experiment!"
    exit 1
fi

func_run_experiment()
{
    # If result already exists, skip the running
    if [ -f ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json ];
    then
        echo ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json exist
        return 0
    fi
    echo START >> ${SCRIPT_HOME}/process

    local WHICH_FILE=$1
    local FILE=$2
    local XY=$3
    local TIME=$4
    local OUTPUTALL=$5

    echo process ${DATA_HOME}stmodel/${WHICH_FILE}/$FILE with x $XY y $XY time $TIME 

    timeout 600s ${OPTKIT_HOME}optkit --stmodel --bgl ${OUTPUTALL}                                                  \
                         -i ${DATA_HOME}stmodel/${WHICH_FILE}/${FILE}                                               \
                         -o ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME} \
                         -c ${XY} -c ${XY} -c ${TIME}                                                               \
                         -l ${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}_

    if [ -f ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json ];
    then
        mv ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json ${DATA_HOME}experiments/results/
        mv ${LOG_HOME}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}_* ${DATA_HOME}experiments/results/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.log
    else
        ${OPTKIT_HOME}optkit --stmodel ${OUTPUTALL}                                                             \
                     -i ${DATA_HOME}stmodel/${WHICH_FILE}/${FILE}                                               \
                     -o ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME} \
                     -c ${XY} -c ${XY} -c ${TIME}                                                               \
                     -l ${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}
        if [ -f ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json ];
        then
            mv ${DATA_HOME}experiments/stmodel/${WHICH_FILE}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.json ${DATA_HOME}experiments/results/
            mv ${LOG_HOME}/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}_* ${DATA_HOME}experiments/results/${WHICH_FILE}_${FILE}_${XY}_${XY}_${TIME}.log
        else
            echo Data is wrong in ${DATA_HOME}stmodel/${WHICH_FILE}/$FILE with x $XY y $XY time $TIME
        fi

    fi

    echo END >> ${SCRIPT_HOME}/process
}

#
# For real crime test data
# Run all crime types
#
func_run_all()
{
    for WHICH_FILE in DC NY CHI
    do
        for XY in 100 200 
        do
            for TIME in 10 30
            do
                for FILE in $(ls  ${DATA_HOME}stmodel/${WHICH_FILE}/); 
                do  
    
                    func_run_experiment ${WHICH_FILE} ${FILE} ${XY} ${TIME} & 
    
                    while true;
                    do
                        NUM_START=$((`cat ${SCRIPT_HOME}/process | grep START | wc -l`))
                        NUM_END=$((`cat ${SCRIPT_HOME}/process | grep END | wc -l`))
                        counter=$((NUM_START- NUM_END))
                        if [[ "$counter" -lt "${NUM_PROC}" ]]
                        then
                            break
                        fi
                        sleep 0.1
                    done
    
                done
            done
        done
    done
}

# run experiment only on core data
func_run_core()
{
    local OUTPUTALL=$1
    for WHICH_FILE in DC NY CHI
    do
        for XY in 100 200 
        do
            for FILE in BURGLARY ROBBERY THEFT 
            do  
                if [ ${XY} == "100" ];
                then
                    TIME=10
                else
                    TIME=30
                fi 
                func_run_experiment ${WHICH_FILE} ${FILE} ${XY} ${TIME} ${OUTPUTALL} & 
    
                while true;
                do
                    NUM_START=$((`cat ${SCRIPT_HOME}/process | grep START | wc -l`))
                    NUM_END=$((`cat ${SCRIPT_HOME}/process | grep END | wc -l`))
                    counter=$((NUM_START- NUM_END))
                    if [[ "$counter" -lt "${NUM_PROC}" ]]
                    then
                        break
                    fi
                    sleep 0.1
                done
    
            done
        done
    done
}

# func_run_core
# func_run_core --notoutputall

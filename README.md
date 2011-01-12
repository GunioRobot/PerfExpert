
# PerfExpert on Ranger
## Setup
Since PerfExpert currently runs from my home dir, you need to copy the following into your `${HOME}/.perfexpert/perfexpert.properties`:
<br/>
<br/>`version = 1.0`
<br/>
<br/>`# Seed configuration file`
<br/>`HPCTOOLKIT_LOCATION = /share/home/01174/ashay/apps/hpctoolkit/`
<br/>`HPCDATA_LOCATION = /share/home/01174/ashay/apps/hpcdata/`
<br/>`CONFIG_LOCATION = /share/home/01174/ashay/apps/PerfExpert/config`
<br/>

## Running PerfExpert
### Regular user
To run your program with PerfExpert, change your jobscript so that the `papi` module is loaded and that the executable name is prefixed by: `~ashay/apps/PerfExpert/perfexpert_run_exp`

For example:
<br/>
<br/>`#!/bin/bash`
<br/>
<br/>`# Sample jobscript to run a.out with PerfExpert`
<br/>
<br/>`#$ -V                     # Inherit the submission environment`
<br/>`#$ -cwd                   # Start job in submission directory`
<br/>`#$ -N sample-job          # Job Name`
<br/>`#$ -j y                   # Combine stderr and stdout`
<br/>`#$ -o $JOB_NAME.o$JOB_ID  # Name of the output file (eg. PerfExpert.oJobID)`
<br/>`#$ -pe 1way 16            # Requests 1 node, 16 cores total`
<br/>`#$ -q normal`
<br/>`#$ -l h_rt=00:10:00       # Typically about 6 to 7 times normal running time`
<br/>
<br/>`################################################################`
<br/>
<br/>`module load papi`
<br/>`~ashay/apps/PerfExpert/perfexpert_run_exp ./source`
<br/>

Also, as you would note, the job execution time now needs to be higher than before. For Ranger, a value between 6x and 7x should suffice. If, at the end of the job, you do not see a 



### Advanced user
If you want to change what happens between multiple runs of your program, use the following instead of using `perfexpert_run_exp`:<br/>
Note that since I could not include backquotes in this document (formatting problem with Markdown), please replace the following script accordingly.

<br/>
<br/>`#!/usr/bin/env bash`
<br/>
<br/>`program_name="./my-executable"`
<br/>`arguments="arg1 arg2 ..."`
<br/>
<br/>`# If there is a pre-generated structure file from hpcstruct, include it here`
<br/>`# OPTIONAL:`
<br/>`hpcstruct_file="path to struct file"`
<br/>
<br/>`#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#`
<br/>`#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#`
<br/>
<br/>`######## Do not edit anything below this line ########`
<br/>
<br/>`#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#`
<br/>`#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#`
<br/>`# Event parameters to each run of hpcrun`
<br/>`experiment[0]="-e PAPI_TOT_CYC:13000027 -e PAPI_TOT_INS:13000049 -e PAPI_TLB_DM:510007 -e PAPI_TLB_IM:510031"`
<br/>`experiment[1]="-e PAPI_TOT_CYC:13000027 -e PAPI_L1_DCA:5300003 -e PAPI_L2_DCA:1300021 -e PAPI_L2_DCM:510007"`
<br/>`experiment[2]="-e PAPI_TOT_CYC:13000027 -e PAPI_L1_ICA:5300003 -e PAPI_L2_ICA:1300021 -e PAPI_L2_ICM:510007"`
<br/>`experiment[3]="-e PAPI_TOT_CYC:13000027 -e PAPI_BR_INS:2300017 -e PAPI_BR_MSP:510007"`
<br/>`experiment[4]="-e PAPI_TOT_CYC:13000027 -e PAPI_FAD_INS:5300003 -e PAPI_FML_INS:5300027 -e PAPI_FDV_INS:510007"`
<br/>
<br/>`# Location of hpcrun, hpcstruct and hpcprof binaries`
<br/>`main_location="${HOME}/.perfexpert/perfexpert.properties"`
<br/>`alt_location="/ops/apps/perfexpert/perfexpert.properties"`
<br/>
<br/>`hpctoolkit_location='(grep "^HPCTOOLKIT_LOCATION" "${main_location}" | sed -e 's/HPCTOOLKIT_LOCATION[\t ]*=[\t ]*\(.*\)/\1/') 2> /dev/null'`
<br/>
<br/>`if [ "x${hpctoolkit_location}" == "x" ]`
<br/>`then`
<br/>`        # Check the home dir`
<br/>`        hpctoolkit_location='(grep "^HPCTOOLKIT_LOCATION" "${alt_location}" | sed -e 's/HPCTOOLKIT_LOCATION[\t ]*=[\t ]*\(.*\)/\1/') 2> /dev/null'`
<br/>`        if [ "x${hpctoolkit_location}" == "x" ]`
<br/>`        then`
<br/>`                echo "Could not find HPCTOOLKIT_LOCATION specified in either \"${main_location}\" or \"${alt_location}\", exiting..."`
<br/>`                exit 1`
<br/>`        fi`
<br/>`fi`
<br/>
<br/>`HPCTOOLKIT_BIN=${hpctoolkit_location}/bin`
<br/>
<br/>`if [ "x${hpcstruct_file}" != "x" ]`
<br/>`then`
<br/>`        if [ ! -f ${hpcstruct_file} ]`
<br/>`        then`
<br/>`                echo "Could not find hpc struct file: ${hpcstruct_file}, try entering the absolute path"`
<br/>`                exit 1`
<br/>`        fi`
<br/>`fi`
<br/>
<br/>`if [ ! -f ${program_name} ]`
<br/>`then`
<br/>`        echo "Could not find file: ${program_name}, try entering the absolute path"`
<br/>`        exit 1`
<br/>`fi`
<br/>
<br/>`# Put everything into a temporary directory instead of clobbering the working directory`
<br/>`tempDir='mktemp -d .perfexpert-temp-XXXXXXX'`
<br/>
<br/>`# Just to be sure, we remove any files which might previously exist`
<br/>`rm -rf ${tempDir}/*`
<br/>
<br/>`# Check if we received the HPCStruct file as a parameter`
<br/>`if [ "x${hpcstruct_file}" != "x" ]`
<br/>`then`
<br/>`        # If yes, use it`
<br/>`        cp ${hpcstruct_file} ${tempDir}/hpcstruct`
<br/>`else`
<br/>`        # Else generate it`
<br/>`        ${HPCTOOLKIT_BIN}/hpcstruct --output ${tempDir}/hpcstruct ${program_name}`
<br/>`fi`
<br/>
<br/>`# Run experiments with different configurations`
<br/>`for index in $(seq 0 $((${#experiment[@]}-1)))`
<br/>`do`
<br/>`        cmd="${HPCTOOLKIT_BIN}/hpcrun ${experiment[${index}]} --output ${tempDir}/measurements ${program_name} ${arguments}"`
<br/>`        eval ${cmd}`
<br/>`done`
<br/>
<br/>`${HPCTOOLKIT_BIN}/hpcprof --metric=thread --struct ${tempDir}/hpcstruct --output ${tempDir}/database ${tempDir}/measurements`
<br/>
<br/>`# Get the final experiment.xml here`
<br/>`mv ${tempDir}/database/experiment.xml ./experiment-$JOB_NAME.o$JOB_ID.xml`
<br/>
<br/>`# Clean up`
<br/>`rm -rf ${tempDir}`
<br/>`rm -f core`
<br/>
<br/>`echo`
<br/>`echo "Run the following command to analyze the measurement data:"`
<br/>`echo "~ashay/apps/PerfExpert/perfexpert <threshold-between-0-and-1> experiment.xml"`
<br/>

# Analysing the collected data
If the run completed successfully, there should be a file by the name: `experiment-${JOB_NAME}.o${JOB_ID}.xml` in the current directory. To analyze this file:
<br/>`$ ~ashay/apps/PerfExpert/perfexpert <threshold-between-0-and-1> <experiment.xml>`
<br/>

# Feedback
If things don't work as expected or if you have suggestions, contact me at: `ashay.rane [at] tacc` or on: `471-4024`

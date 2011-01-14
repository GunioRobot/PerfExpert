
# PerfExpert on Ranger
## Setup
Since PerfExpert currently runs from my home dir, you need to copy the following into your `${HOME}/.perfexpert/perfexpert.properties`:
    
    # Seed configuration file
    version = 1.0
    
    HPCTOOLKIT_LOCATION = /share/home/01174/ashay/apps/hpctoolkit/
    HPCDATA_LOCATION = /share/home/01174/ashay/apps/hpcdata/
    CONFIG_LOCATION = /share/home/01174/ashay/apps/PerfExpert/config
    

## Running PerfExpert
### Regular user
To run your program with PerfExpert, change your jobscript so that the `papi` module is loaded and that the executable name is prefixed by: `~ashay/apps/PerfExpert/perfexpert_run_exp`

For example:
   
    #!/bin/bash
   
    # Sample jobscript to run a.out with PerfExpert
   
    #$ -V                     # Inherit the submission environment
    #$ -cwd                   # Start job in submission directory
    #$ -N sample-job          # Job Name
    #$ -j y                   # Combine stderr and stdout
    #$ -o $JOB_NAME.o$JOB_ID  # Name of the output file (eg. PerfExpert.oJobID)
    #$ -pe 1way 16            # Requests 1 node, 16 cores total
    #$ -q normal
    #$ -l h_rt=00:10:00       # Typically about 6 to 7 times normal running time
   
    ################################################################
   
    module load papi
    ~ashay/apps/PerfExpert/perfexpert_run_exp ./source
   

Also, as you would note, the job execution time now needs to be higher than before. For Ranger, a value between 6x and 7x should suffice. If, at the end of the job, you do not see a file: `experiment-${JOB_NAME}.o${JOB_ID}.xml`, then it could be likely that the runtime for the jobscript was not sufficient.



### Advanced user
If you want to change what happens between multiple runs of your program, use the following instead of using `perfexpert_run_exp`:<br/>

   
    program_name="./my-executable"
    arguments="arg1 arg2 ..."
   
    # If there is a pre-generated structure file from hpcstruct, paste the absolute path to it in the variable below
    # OPTIONAL:
    hpcstruct_file=""
   
    #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
    #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
   
    ######## Do not edit anything below this line ########
   
    #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
    #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
    # Event parameters to each run of hpcrun
    experiment[0]="--event PAPI_TOT_CYC:13000027 --event PAPI_TOT_INS:13000049 --event PAPI_TLB_DM:510007 --event PAPI_TLB_IM:510031"
    experiment[1]="--event PAPI_TOT_CYC:13000027 --event PAPI_L1_DCA:5300003 --event PAPI_L2_DCA:1300021 --event PAPI_L2_DCM:510007"
    experiment[2]="--event PAPI_TOT_CYC:13000027 --event PAPI_L1_ICA:5300003 --event PAPI_L2_ICA:1300021 --event PAPI_L2_ICM:510007"
    experiment[3]="--event PAPI_TOT_CYC:13000027 --event PAPI_BR_INS:2300017 --event PAPI_BR_MSP:510007"
    experiment[4]="--event PAPI_TOT_CYC:13000027 --event PAPI_FAD_INS:5300003 --event PAPI_FML_INS:5300027 --event PAPI_FDV_INS:510007"
   
    # Location of hpcrun, hpcstruct and hpcprof binaries
    main_location="${HOME}/.perfexpert/perfexpert.properties"
    alt_location="/opt/apps/perfexpert/perfexpert.properties"
   
    hpctoolkit_location=`(grep "^HPCTOOLKIT_LOCATION" "${main_location}" | sed -e 's/HPCTOOLKIT_LOCATION[\t ]*=[\t ]*\(.*\)/\1/') 2> /dev/null`
   
    if [ "x${hpctoolkit_location}" == "x" ]
    then
            # Check the home dir
            hpctoolkit_location=`(grep "^HPCTOOLKIT_LOCATION" "${alt_location}" | sed -e 's/HPCTOOLKIT_LOCATION[\t ]*=[\t ]*\(.*\)/\1/') 2> /dev/null`
            if [ "x${hpctoolkit_location}" == "x" ]
            then
                    echo "Could not find HPCTOOLKIT_LOCATION specified in either \"${main_location}\" or \"${alt_location}\", exiting..."
                    exit 1
            fi
    fi
   
    HPCTOOLKIT_BIN=${hpctoolkit_location}/bin
   
    if [ "x${hpcstruct_file}" != "x" ]
    then
            if [ ! -f ${hpcstruct_file} ]
            then
                    echo "Could not find hpc struct file: ${hpcstruct_file}, try entering the absolute path"
                    exit 1
            fi
    fi
   
    if [ ! -f ${program_name} ]
    then
            echo "Could not find file: ${program_name}, try entering the absolute path"
            exit 1
    fi
   
    # Put everything into a temporary directory instead of clobbering the working directory
    tempDir=`mktemp -d .perfexpert-temp-XXXXXXX`
   
    # Just to be sure, we remove any files which might previously exist
    rm -rf ${tempDir}/*
   
    # Check if we received the HPCStruct file as a parameter
    if [ "x${hpcstruct_file}" != "x" ]
    then
            # If yes, use it
            cp ${hpcstruct_file} ${tempDir}/hpcstruct
    else
            # Else generate it
            ${HPCTOOLKIT_BIN}/hpcstruct --output ${tempDir}/hpcstruct ${program_name}
    fi
   
    # Run experiments with different configurations
    for index in $(seq 0 $((${#experiment[@]}-1)))
    do
            cmd="${HPCTOOLKIT_BIN}/hpcrun ${experiment[${index}]} --output ${tempDir}/measurements ${program_name} ${arguments}"
            eval ${cmd}
    done
   
    ${HPCTOOLKIT_BIN}/hpcprof --metric=thread --struct ${tempDir}/hpcstruct --output ${tempDir}/database ${tempDir}/measurements
   
    # Get the final experiment.xml here
    mv ${tempDir}/database/experiment.xml ./experiment-$JOB_NAME.o$JOB_ID.xml
   
    # Clean up
    rm -rf ${tempDir}
    rm -f core
   
    echo
    echo "Run the following command to analyze the measurement data:"
    echo "~ashay/apps/PerfExpert/perfexpert <threshold-between-0-and-1> experiment.xml"
   

# Analysing the collected data
If the run completed successfully, there should be a file by the name: `experiment-${JOB_NAME}.o${JOB_ID}.xml` in the current directory. To analyze this file:
    $ module load java
    $ ~ashay/apps/PerfExpert/perfexpert <threshold-between-0-and-1> <experiment.xml>
   

# Feedback
If things don't work as expected or if you have suggestions, contact me at: `ashay.rane [at] tacc` or on: `471-4024`. Please try to include the following in your report:
*   experiment.xml file that is being passed to PerfExpert
*   Error printed on the console
*   perfexpert.log (present in the directory from where `~ashay/apps/PerfExpert/perfexpert` was run

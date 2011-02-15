#!/bin/bash

# Uses ${HPCTOOLKIT_HOME} and ${HPCDATA_HOME} environment variables

if [ "x${HPCTOOLKIT_HOME}" == "x" ]
then
	read -p "HPCToolkit location (absolute path): " HPCTOOLKIT_HOME
fi

if [ "x${HPCDATA_HOME}" == "x" ]
then
	read -p "HPCData location (absolute path): " HPCDATA_HOME
fi

if [ ! -f ${HPCTOOLKIT_HOME}/bin/hpcrun -o ! -f ${HPCTOOLKIT_HOME}/bin/hpcstruct -o ! -f ${HPCTOOLKIT_HOME}/bin/hpcprof ]
then
	echo "Could not locate HPCToolkit binaries (hpcrun, hpcstruct, hpcprof) in ${HPCTOOLKIT_HOME}/bin, did you set/input HPCTOOLKIT_HOME correctly?"
	exit 1
fi

if [ ! -f ${HPCDATA_HOME}/hpcdata.sh ]
then
	echo "Could not locate HPCData script (hpcdata.sh) in ${HPCDATA_HOME}, did you set/input HPCDATA_HOME correctly?"
	exit 1
fi

cat > perfexpert.properties << END_HERE
version = 1.0

# Generated as part of installation
HPCTOOLKIT_LOCATION = ${HPCTOOLKIT_HOME}
HPCDATA_LOCATION = ${HPCDATA_HOME}
END_HERE

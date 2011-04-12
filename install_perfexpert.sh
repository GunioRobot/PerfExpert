#!/bin/bash

# Uses ${PERFEXPERT_HOME} and ${PERFEXPERT_GLOBAL_INSTALL}

# First check if all required files are present
if [ ! -f ./perfexpert_run_exp -o ! -f ./config/machine.properties -o ! -f ./config/lcpi.properties -o ! -f ./bin/perfexpert.jar -o ! -f ./perfexpert ]
then
	echo "One of the following files was not found, did \`make all' run without errors?"
	echo -e "`pwd`/perfexpert_run_exp"
	echo -e "`pwd`/config/machine.properties"
	echo -e "`pwd`/config/lcpi.properties"
	echo -e "`pwd`/bin/perfexpert.jar"
	echo -e "`pwd`/perfexpert"
	exit 1
fi

trimmed=`echo ${PERFEXPERT_HOME} | sed 's/ //g' | sed 's/\///g'`
if [ "x${trimmed}" == "x" ]
then
	# Try system-wide installation first
	PERFEXPERT_HOME=/opt/apps/perfexpert
	echo "The shell variable \${PERFEXPERT_HOME} was not set or was set to an invalid value, defaulting to /opt/apps/perfexpert"
else
	echo "Using shell variable \${PERFEXPERT_HOME} set to \"${PERFEXPERT_HOME}\""
fi

# For making this work while building RPMs
PERFEXPERT_HOME="${RPM_BUILD_ROOT}${PERFEXPERT_HOME}"

# If path does not exist, create it
if [ ! -d "${PERFEXPERT_HOME}" ]
then
	mkdir -p "${PERFEXPERT_HOME}"
	if [ ${?} != 0 ]
	then
		echo "Failed creating directory: \"${PERFEXPERT_HOME}\", exiting..."
		exit 1
	fi
fi

# Empty out the existing contents
trimmed=`echo ${PERFEXPERT_HOME} | sed 's/ //g' | sed 's/\///g'`
if [ "x${trimmed}" != "x" ]
then
	rm -rf "${PERFEXPERT_HOME}/"
	mkdir "${PERFEXPERT_HOME}"
else
	echo "\${PERFEXPERT_HOME} was not set or was set to an invalid path..."
	exit 1
fi

# Copy files into the correct locations
mkdir "${PERFEXPERT_HOME}/config"
mkdir "${PERFEXPERT_HOME}/bin"
mkdir "${PERFEXPERT_HOME}/lib"
install ./perfexpert_run_exp "${PERFEXPERT_HOME}/perfexpert_run_exp"
install ./config/machine.properties "${PERFEXPERT_HOME}/config/machine.properties"
install ./config/lcpi.properties "${PERFEXPERT_HOME}/config/lcpi.properties"
install ./bin/perfexpert.jar "${PERFEXPERT_HOME}/bin/perfexpert.jar"
install ./perfexpert "${PERFEXPERT_HOME}/perfexpert"
install ./lib/log4j-1.2.16.jar "${PERFEXPERT_HOME}/lib/log4j-1.2.16.jar"
install ./lib/log4j.properties "${PERFEXPERT_HOME}/lib/log4j.properties"

echo -e "\nPerfExpert installation is complete! You can now use PerfExpert from \"${PERFEXPERT_HOME}\".\nFor usage guide, please refer: http://www.tacc.utexas.edu/perfexpert/perfexpert-usage-guide/"

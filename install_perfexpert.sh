#!/bin/bash

# Uses ${PERFEXPERT_HOME} and ${PERFEXPERT_GLOBAL_INSTALL}

# First check if all required files are present
if [ ! -f ./perfexpert_run_exp -o ! -f ./config/machine.properties -o ! -f ./config/lcpi.properties -o ! -f ./bin/perfexpert.jar -o ! -f ./perfexpert.properties -o ! -f ./perfexpert ]
then
	echo "One of the following files was not found, did \`make all' run without errors?"
	echo -e "`pwd`/perfexpert_run_exp"
	echo -e "`pwd`/config/machine.properties"
	echo -e "`pwd`/config/lcpi.properties"
	echo -e "`pwd`/bin/perfexpert.jar"
	echo -e "`pwd`/perfexpert.properties"
	echo -e "`pwd`/perfexpert"
	exit 1
fi

if [ "x${PERFEXPERT_HOME}" == "x" ]
then
	# Try system-wide installation first
	PERFEXPERT_HOME=/opt/apps/perfexpert
	echo "The shell variable \${PERFEXPERT_HOME}  was not set, defaulting to /opt/apps/perfexpert"
	# read -p "Directory to install PerfExpert in: (absolute path) " PERFEXPERT_HOME
else
	echo "Using shell variable \${PERFEXPERT_HOME} set to \"${PERFEXPERT_HOME}\""
fi

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
if [ "x${PERFEXPERT_HOME}" != "x" ]
then
	rm -rf "${PERFEXPERT_HOME}/"
	mkdir "${PERFEXPERT_HOME}"
else
	echo "\${PERFEXPERT_HOME} was not set, weird..."
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

if [ "x${PERFEXPERT_GLOBAL_INSTALL}" == "x" ]
then
	echo "The shell variable \${PERFEXPERT_GLOBAL_INSTALL} was not set to either \"Yes\" or \"No\", defaulting to \"Yes\""
	PROPERTIES_HOME="/opt/apps/perfexpert"

#	read -p "Install system-wide, for all users? (requires write access to /opt/apps) [Y/n]: " global
#
#	# If its empty, set it to "yes"
#	if [ "x${global}" == "x" ]
#	then
#		global="Y"
#	fi
#
#	if [ ${global:0:1} == "n" -o ${global:0:1} == "N" ]
#	then
#		PROPERTIES_HOME="${HOME}/.perfexpert"
#	else
#		PROPERTIES_HOME="/opt/apps/perfexpert"
#	fi
else
	echo "Using shell variable \${PERFEXPERT_GLOBAL_INSTALL} set to \"${PERFEXPERT_GLOBAL_INSTALL}\""
	if [ "${PERFEXPERT_GLOBAL_INSTALL:0:1}" == "Y" -o "${PERFEXPERT_GLOBAL_INSTALL:0:1}" == "y" -o "${PERFEXPERT_GLOBAL_INSTALL}" == "true" -o "${PERFEXPERT_GLOBAL_INSTALL}" == "True" ]
	then
		PROPERTIES_HOME="/opt/apps/perfexpert"
	else
		PROPERTIES_HOME="${HOME}/.perfexpert"
	fi
fi

# Create directory if it does not exist
if [ ! -d "${PROPERTIES_HOME}" ]
then
	mkdir -p "${PROPERTIES_HOME}"
	if [ ${?} != "0" ]
	then
		echo "Error creating directory: \"${PROPERTIES_HOME}\", exiting..."
		exit 1
	fi
fi

install ./perfexpert.properties "${PROPERTIES_HOME}/perfexpert.properties"

echo -e "\nPerfExpert installation is complete! You can now use PerfExpert from \"${PERFEXPERT_HOME}\".\nFor usage guide, please refer: https://webspace.utexas.edu/asr596/www/perfexpert/blog/2011/02/07/PerfExpert-Usage-Guide.html"

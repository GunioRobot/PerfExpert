#
# perfexpert-2.0.6.spec, v3.0, 2011-02-24 11:59:00 carlos@tacc.utexas.edu
#
# See http://www.tacc.utexas.edu/perfexpert/

Summary:    Performance Bottleneck Remediation Tool
Name:       perfexpert
Version:    2.0.6
Release:    1
License:    LGPLv3
Vendor:     The University of Texas at Austin
Group:      Applications/HPC
Source:     %{name}-%{version}.tar.gz
Packager:   TACC - carlos@tacc.utexas.edu, TACC - ashay.rane@tacc.utexas.edu
# This is the actual installation directory - Careful
BuildRoot:  /var/tmp/%{name}-%{version}-buildroot

#------------------------------------------------
# BASIC DEFINITIONS
#------------------------------------------------
%define system linux
#This is for Lonestar and Longhorn
#define _topdir /home/build/rpms/
#This is for Ranger only
%define _topdir /share/home/0000/build/rpms
%define APPS    /opt/apps
%define MODULES modulefiles

# Allow for creation of multiple packages with this spec file
# Any tags right after this line apply only to the subpackage
# Summary and Group are required.
#package -n %{name}
Summary: Performance Bottleneck Remediation Tool
Group:   Applications/HPC

#------------------------------------------------
# PACKAGE DESCRIPTION
#------------------------------------------------
%description
%description -n %{name}
PerfExpert is a performance analysis tool that provides concise 
performance assessment and suggests steps that can be taken by 
the developer to improve performance.

#------------------------------------------------
# INSTALLATION DIRECTORY
#------------------------------------------------
# Buildroot: defaults to null if not included here
%define INSTALL_DIR %{APPS}/%{name}/%{version}
%define MODULE_DIR  %{APPS}/%{MODULES}/%{name}
%define HPCTOOLKIT_INSTALL_DIR %{APPS}/%{name}/%{version}/extras/hpctoolkit
%define PERFEXPERT_HPCTOOLKIT_HOME %{INSTALL_DIR}/extras/hpctoolkit
%define PERFEXPERT_HPCDATA_HOME %{INSTALL_DIR}/extras/hpcdata-r754

#------------------------------------------------
# PREPARATION SECTION
#------------------------------------------------
# Use -n <name> if source file different from <name>-<version>.tar.gz
%prep

# Remove older attempts
rm   -rf $RPM_BUILD_ROOT/%{INSTALL_DIR}
mkdir -p $RPM_BUILD_ROOT/%{INSTALL_DIR}

# Unpack source
# This will unpack the source to /tmp/BUILD/name-version
%setup -n %{name}-%{version}

#------------------------------------------------
# BUILD SECTION
#------------------------------------------------
%build
# Use mount temp trick
 mkdir -p             %{INSTALL_DIR}
 mount -t tmpfs tmpfs %{INSTALL_DIR}

# Start with a clean environment
if [ -f "$BASH_ENV" ]; then
   . $BASH_ENV
   export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
   export PERFEXPERT_HOME=%{INSTALL_DIR}
   export PERFEXPERT_HPCTOOLKIT_HOME=%{INSTALL_DIR}/extras/hpctoolkit
   export PERFEXPERT_HPCDATA_HOME=%{INSTALL_DIR}/extras/hpcdata-r754
else
   setenv PERFEXPERT_HOME %{INSTALL_DIR}
   setenv PERFEXPERT_HPCTOOLKIT_HOME %{INSTALL_DIR}/extras/hpctoolkit
   setenv PERFEXPERT_HPCDATA_HOME %{INSTALL_DIR}/extras/hpcdata-r754
fi
module purge
module load TACC
module unload CTSSV4 mvapich pgi
module load gcc jdk64 papi


#
# HPCTOOLKIT MUST BE BUILT FOR PERFEXPERT
#
cd ./extras/hpctoolkit-externals-r3572
./configure
make all
make distclean

cd ../hpctoolkit-r3572
./configure --prefix=%{HPCTOOLKIT_INSTALL_DIR} --with-externals=../hpctoolkit-externals-r3572 --with-papi=$TACC_PAPI_DIR
make

#------------------------------------------------
# INSTALL SECTION
#------------------------------------------------
%install

# Start with a clean environment
if [ -f "$BASH_ENV" ]; then
   . $BASH_ENV
   export MODULEPATH=/opt/apps/teragrid/modulefiles:/opt/apps/modulefiles:/opt/modulefiles
   export PERFEXPERT_HOME=%{INSTALL_DIR}
   export PERFEXPERT_HPCTOOLKIT_HOME=%{INSTALL_DIR}/extras/hpctoolkit
   export PERFEXPERT_HPCDATA_HOME=%{INSTALL_DIR}/extras/hpcdata-r754
else
   setenv PERFEXPERT_HOME %{INSTALL_DIR}
   setenv PERFEXPERT_HPCTOOLKIT_HOME %{INSTALL_DIR}/extras/hpctoolkit
   setenv PERFEXPERT_HPCDATA_HOME %{INSTALL_DIR}/extras/hpcdata-r754
fi
module purge
module load TACC
module unload CTSSV4 mvapich pgi
module load gcc jdk64 papi

 mkdir -p $RPM_BUILD_ROOT/%{INSTALL_DIR}

# Install the hpctoolkit build and get back to the original 
# directory
 cd ./extras/hpctoolkit-r3572
 make install
 cd ../../

 cp -r ./extras/hpcdata-r754 %{INSTALL_DIR}/extras

# Substitute hardcoded path to develeper's home directory
# by installation directory (/share/home/... for Ranger, /home/... for Longhorn)
# sed -i 's+/home/00976/burtsche/PerfExpert+/opt/apps/perfexpert/1.0.1+' ./PerfExpert.perl

# Now install perfexpert itself
 make
 make install

#  Kluge, the make install, installs in /tmp/carlos
cp    -r %{INSTALL_DIR}/ $RPM_BUILD_ROOT/%{INSTALL_DIR}/..
umount                                   %{INSTALL_DIR}


# ADD ALL MODULE STUFF HERE
# TACC module

mkdir -p $RPM_BUILD_ROOT/%{MODULE_DIR}
cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/%{version} << 'EOF'
#%Module1.0####################################################################
##
## PerfExpert
##
proc ModulesHelp { } {
        puts stderr "\n"
	puts stderr "\tThe perfexpert module makes TACC_PERFEXPERT_DIR"
	puts stderr "\tavailable, and adds the PerfExpert directory to your PATH."
        puts stderr "\n"
        puts stderr "For a detailed description of PerfExpert visit:"
        puts stderr "http://www.tacc.utexas.edu/perfexpert/"
	puts stderr "\n"
	puts stderr "\tVersion %{version}\n"
}

module-whatis "PerfExpert"
module-whatis "Version: %{version}"
module-whatis "Category: application, hpc"
module-whatis "Description: Performance Bottleneck Remediation Tool"
module-whatis "URL: http://www.tacc.utexas.edu/perfexpert/"

# Prerequisites
prereq papi java

# Tcl script only
set version %{version}

# Export environmental variables
setenv TACC_PERFEXPERT_DIR %{INSTALL_DIR}
setenv TACC_PERFEXPERT_BIN %{INSTALL_DIR}
setenv PERFEXPERT_HPCTOOLKIT_HOME %{PERFEXPERT_HPCTOOLKIT_HOME}
setenv PERFEXPERT_HPCDATA_HOME %{PERFEXPERT_HPCDATA_HOME}

# Prepend the scalasca directories to the adequate PATH variables
prepend-path PATH %{INSTALL_DIR}

# This is only necessary if there will be submodules built on 
# this package. Not the case with Scalasca (for the time being).
# prepend-path MODULEPATH %{MODULE_DIR}
EOF

cat > $RPM_BUILD_ROOT/%{MODULE_DIR}/.version.%{version} << 'EOF'
#%Module1.0####################################################################
##
## Version file for perfexpert version %{version}
##
set ModulesVersion "%version"
EOF

#------------------------------------------------
# FILES SECTION
#------------------------------------------------
%files -n %{name}

# Define files permisions, user and group
%defattr(-,root,install)
%{INSTALL_DIR}
%{MODULE_DIR}

#------------------------------------------------
# CLEAN UP SECTION
#------------------------------------------------
%post
%clean
# Make sure we are not within one of the directories we try to delete
cd /tmp

# Remove the source files from /tmp/BUILD
rm -rf /tmp/BUILD/%{name}-%{version}

# Remove the installation files now that the RPM has been generated
rm -rf /var/tmp/%{name}-%{version}-buildroot


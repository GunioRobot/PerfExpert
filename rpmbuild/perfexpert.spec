Summary: Expert system to identify bottlenecks in code and resolve them
Name: perfexpert
Version: 2.0.1
Release: 1
License: LGPL
Group: Development/Tools
Source: /home/01174/ashay/rpm/SOURCES/perfexpert-2.0.1.tar.gz
BuildRoot: /tmp/%{name}-buildroot

%description
Expert system to identify bottlenecks in code and resolve them

%prep
%setup -q

%build
make RPM_OPT_FLAGS="${RPM_OPT_FLAGS}"

%install
make install

%files
%defattr(-,-,-)

/opt/apps/perfexpert/bin/perfexpert.jar
/opt/apps/perfexpert/config/lcpi.properties
/opt/apps/perfexpert/config/machine.properties
/opt/apps/perfexpert/lib/log4j-1.2.16.jar
/opt/apps/perfexpert/lib/log4j.properties
/opt/apps/perfexpert/perfexpert
/opt/apps/perfexpert/perfexpert.properties
/opt/apps/perfexpert/perfexpert_run_exp


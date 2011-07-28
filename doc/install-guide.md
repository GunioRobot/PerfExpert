CSS: style.css

PerfExpert Installation Instructions
====================================

Prerequisites
-------------

PerfExpert is based on other tools and thus requires that they be installed. These are:

* **PAPI** (<http://icl.cs.utk.edu/papi/software/index.html>): PAPI is required to measure hardware performance metrics like cache misses, branch instructions, etc. The PAPI installation is mostly straight-forward: Download, configure, make and make install. If your Linux kernel version is 2.6.32 or higher, then PAPI will mostly likely use `perf_events`. Recent versions of PAPI (v3.7.2 and beyond) support using `perf_events` in the Linux kernel. However, if your kernel version is lower than 2.6.32, then you would require patching the kernel with either `perfctr` (<http://user.it.uu.se/~mikpe/linux/perfctr/2.6/>) or `perfmon` (<http://perfmon2.sourceforge.net/>).

* **HPCToolkit** (<http://hpctoolkit.org/software.html>): HPCToolkit is a tool that works on top of PAPI. HPCToolkit is used by PerfExpert to run the program multiple times with specific performance counters enabled. It is also useful for correlating addresses in the compiled binary back to the source code. HPCToolkit is comprised of three parts:

  1.  `hpctoolkit` (required by PerfExpert): Which contains the programs to run the user application with performance counters and to generate the profiles
  2.  `hpctoolkit-externals`: External libraries (e.g. `libdwarf`, `libxml2`, etc.) that are used by `hpctoolkit`
  3.  `hpcdata`: Which is required for converting the experiment file generated using HPCToolkit. Available here: <https://webspace.utexas.edu/asr596/www/perfexpert/downloads/hpcdata.tar.gz>.

* **Java Development Kit** (<http://www.oracle.com/technetwork/java/javase/downloads/index.html>) and **Apache Ant** (<http://ant.apache.org/bindownload.cgi>): To compile PerfExpert


Downloading PerfExpert
----------------------

The PerfExpert source code can be downloaded from: <https://github.com/ashay/PerfExpert/archives/master>. If you would like to keep up-to-date with the source code, you can fetch the code from the git repository here: <https://github.com/ashay/PerfExpert>. For directions on how to use git, see <http://git-scm.com/documentation>.


Setting up PerfExpert
---------------------

PerfExpert now comes with a Makefile to automate the entire installation process. The installation process requires a few user inputs which are supplied via environment variables:

* `PAPI_HEADERS`: Absolute path to `papi.h` (optional if PAPI is installed in the standard location - `/usr`)
* `PAPI_LIBS`: Absolute path to `libpapi.a` or `libpapi.so` (optional, see `PAPI_HEADERS`)
* `PERFEXPERT_HOME`: Absolute path to location where PerfExpert should be installed (default: `/opt/apps/perfexpert`)

Once the required environment variables are set, run `make` followed by `make install`. As a final step of installing PerfExpert, the following two environment variables need to be set:
* `PERFEXPERT_HPCTOOLKIT_HOME` to the absolute path where HPCToolkit was installed
* `PERFEXPERT_HPCDATA_HOME` to the absolute path where the `hpcdata` folder was extracted

The following is a sample installation on my laptop.

    $ export PERFEXPERT_HOME=/home/klaus/apps/perfexpert
     
    $ make
    `which ant` -f ./build/build.xml jar
    Buildfile: /home/klaus/.workspace/PerfExpert/build/build.xml
     
    clean:
       [delete] Deleting directory /home/klaus/.workspace/PerfExpert/bin
     
    init:
        [mkdir] Created dir: /home/klaus/.workspace/PerfExpert/bin
         [copy] Copying 4 files to /home/klaus/.workspace/PerfExpert/bin
     
    build-project:
         [echo] PerfExpert: /home/klaus/.workspace/PerfExpert/build/build.xml
        [javac] /home/klaus/.workspace/PerfExpert/build/build.xml:33: warning: 'includeantruntime' was not set,
                    defaulting to build.sysclasspath=last; set to false for repeatable builds
        [javac] Compiling 20 source files to /home/klaus/.workspace/PerfExpert/bin
     
    jar:
          [jar] Building jar: /home/klaus/.workspace/PerfExpert/bin/perfexpert.jar
     
    BUILD SUCCESSFUL
    Total time: 1 second
    cd hound/ && make && cd ..
    make[1]: Entering directory `/home/klaus/.workspace/PerfExpert/hound'
    rm -f bin/hound src/driver.o
    rm -rf bin/
    cc -m32 -fPIC   -c -o src/driver.o src/driver.c
    mkdir bin/
    cc -m32 -fPIC src/driver.o -o bin/hound
    make[1]: Leaving directory `/home/klaus/.workspace/PerfExpert/hound'
    mkdir config/
    cd hound/ && ./bin/hound | tee machine.properties && mv machine.properties ../config/machine.properties && cd ..
    version = 1.0
     
    # Generated using hound
    CPI_threshold = 0.5
    L1_dlat = 3.75
    L1_ilat = 3.75
    L2_lat = 20.09
    mem_lat = 249.52
    CPU_freq = 2401000000
    FP_lat = 3.61
    FP_slow_lat = 22.53
    BR_lat = 1.00
    BR_miss_lat = 4.36
    TLB_lat = 15.63
    cd sniffer/ && make CFLAGS="" LDFLAGS="-lpapi " && cd ..
    make[1]: Entering directory `/home/klaus/.workspace/PerfExpert/sniffer'
    rm -f bin/sniffer src/driver.o
    rm -f lcpi.properties experiment.header.tmp
    rm -rf bin/
    cc    -c -o src/driver.o src/driver.c
    mkdir bin/
    cc -lpapi  src/driver.o -o bin/sniffer
    make[1]: Leaving directory `/home/klaus/.workspace/PerfExpert/sniffer'
    mkdir config/
    mkdir: cannot create directory `config/': File exists
    make: [sniffer] Error 1 (ignored)
    cd sniffer/ && ./bin/sniffer && mv lcpi.properties ../config/lcpi.properties && ./patch_run.sh && cd ..
    Generated LCPI and experiment files
     
    $ make install
    ./install_perfexpert.sh
    Using shell variable ${PERFEXPERT_HOME} set to "/home/klaus/apps/perfexpert"
     
    PerfExpert installation is complete! You can now use PerfExpert from "/home/klaus/apps/perfexpert".
    For usage guide, please refer: http://www.tacc.utexas.edu/perfexpert/perfexpert-usage-guide
    
    $ echo "export PERFEXPERT_HPCTOOLKIT_HOME=${HOME}/apps/hpctoolkit" >> ~/.bashrc
    $ echo "export PERFEXPERT_HPCDATA_HOME=${HOME}/apps/hpcdata" >> ~/.bashrc

* * *

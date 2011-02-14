
ANT_LOCATION=`which ant`

# PAPI_HEADERS and PAPI_LIBS are passed in via environment variables

ifneq ($(strip $(PAPI_HEADERS)),)
SNIFFER_CFLAGS=-I $(PAPI_HEADERS)
endif

SNIFFER_LDFLAGS=-lpapi $(PAPI_LD_FLAGS)
ifneq ($(strip $(PAPI_LIBS)),)
SNIFFER_LDFLAGS+=-L $(PAPI_LIBS)
endif

all:	jar hound sniffer paths

jar:
	$(ANT_LOCATION) -f ./build/build.xml jar

hound:	build-hound
	-mkdir config/
	cd hound/ && ./bin/hound | tee machine.properties && mv machine.properties ../config/machine.properties && cd ..

build-hound:
	cd hound/ && make && cd ..

sniffer:	build-sniffer
	-mkdir config/
	cd sniffer/ && ./bin/sniffer && mv lcpi.properties ../config/lcpi.properties && ./patch_run.sh && cd ..

build-sniffer:
	cd sniffer/ && make CFLAGS="$(SNIFFER_CFLAGS)" LDFLAGS="$(SNIFFER_LDFLAGS)" && cd ..

paths:
	./build-paths.sh

install:
	./install_perfexpert.sh

clean:
	rm -f ./bin/perfexpert.jar
	cd hound/ && make clean && cd ..
	rm -f ./hound/machine.properties
	cd sniffer/ && make clean && cd ..
	rm -f ./sniffer/lcpi.properties ./sniffer/experiment.header.tmp

distclean:	clean
	rm -f ./perfexpert_run_exp ./config/machine.properties ./config/lcpi.properties ./perfexpert.properties

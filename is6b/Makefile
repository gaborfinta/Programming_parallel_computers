bin=is-test is-benchmark pngsegment
include ../common/Makefile.cuda

SOURCES+=./../is-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../is-common -I ./../common -lpng
LDFLAGS+=-lpng
vpath %.h ../is-common:../common
vpath %.cc ../is-common:../common

is.o: is.cu
	$(NVCC) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

pngsegment: pngsegment.o is.o pngio.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+ -lpng

is-test: is-test.o is.o error.o pngio.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+ -lpng

is-benchmark: is-benchmark.o is.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+

include Makefile.dep

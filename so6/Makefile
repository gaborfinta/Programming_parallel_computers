bin=so-test so-benchmark
include ../common/Makefile.cuda

SOURCES+=./../so-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../so-common -I ./../common
vpath %.h ../so-common:../common
vpath %.cc ../so-common:../common

so.o: so.cu
	$(NVCC) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

so-test: so-test.o so.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) $^ -o $@

so-benchmark: so-benchmark.o so.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) $^ -o $@

include Makefile.dep

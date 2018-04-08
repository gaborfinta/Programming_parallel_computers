bin=nn nn-test nn-benchmark
include ../common/Makefile.cuda

SOURCES+=./../nn-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../cp-common -I ./../common
vpath %.h ../nn-common:../common
vpath %.cc ../nn-common:../common

nn.o: nn.cu
	$(NVCC) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

nn: nn.o nn-main.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+

nn-test: nn.o nn-test.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+

nn-benchmark: nn.o nn-benchmark.o error.o
	$(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+

include Makefile.dep


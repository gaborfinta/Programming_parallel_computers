bin=nn nn-test nn-benchmark
include ../common/Makefile.common

SOURCES+=./../nn-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../nn-common -I ./../common -fopenmp
LDFLAGS+=-fopenmp
vpath %.h ../nn-common:../common
vpath %.cc ../nn-common:../common

nn: nn.o nn-main.o
	$(CXX) $(LDFLAGS) $^ -o $@

nn-test: nn.o nn-test.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

nn-benchmark: nn.o nn-benchmark.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

include Makefile.dep

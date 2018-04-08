bin=so-test so-benchmark
include ../common/Makefile.common

SOURCES+=./../so-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../so-common -I ./../common -fopenmp
LDFLAGS+=-fopenmp
vpath %.h ../so-common:../common
vpath %.cc ../so-common:../common

so-test: so-test.o so.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

so-benchmark: so-benchmark.o so.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

include Makefile.dep

bin=is-test is-benchmark pngsegment
include ../common/Makefile.common

SOURCES+=./../is-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../is-common -I ./../common -fopenmp -lpng
LDFLAGS+=-lpng -fopenmp
vpath %.h ../is-common:../common
vpath %.cc ../is-common:../common

pngsegment: pngsegment.o is.o pngio.o error.o
	$(CXX) $^ $(LDFLAGS)  -o $@

is-test: is-test.o is.o error.o pngio.o
	$(CXX) $^ $(LDFLAGS)  -o $@

is-benchmark: is-benchmark.o is.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

include Makefile.dep

bin=pngmf mf-test mf-benchmark
include ../common/Makefile.common

SOURCES+=./../mf-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../mf-common -I ./../common -fopenmp
LDFLAGS+=-fopenmp
vpath %.h ../mf-common:../common
vpath %.cc ../mf-common:../common


pngmf: pngmf.o mf.o pngio.o error.o
	$(CXX) $(LDFLAGS) $^ -lpng -o $@

mf-test: mf-test.o mf.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

mf-benchmark: mf-benchmark.o mf.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

include Makefile.dep

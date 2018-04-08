bin=pngcorrelate cp-test cp-benchmark
include ../common/Makefile.common

SOURCES+=./../cp-common/*.cc ./../common/*.cc
CXXFLAGS+=-I ./ -I ./../cp-common -I ./../common
vpath %.h ../cp-common:../common
vpath %.cc ../cp-common:../common

pngcorrelate: pngcorrelate.o cp.o pngio.o error.o
	$(CXX) $(LDFLAGS) $^ -lpng -o $@

cp-test: cp-test.o cp.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

cp-benchmark: cp-benchmark.o cp.o error.o
	$(CXX) $(LDFLAGS) $^ -o $@

include Makefile.dep

bin=pngdiff
include ../common/Makefile.common

pngdiff: pngdiff.o pngio.o error.o
	$(CXX) $(LDFLAGS) $^ -lpng -o $@

include Makefile.dep

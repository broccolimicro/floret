CXXFLAGS     = -g -O2 -Wall -fmessage-length=0 -I. -L. -Ideps/gdstk/include -Ideps/ruler -Ideps/pgen -Ldeps/gdstk/build/install/lib -Ldeps/gdstk/build/install/lib64  -Ldeps/ruler -Ldeps/pgen
# -g -fprofile-arcs -ftest-coverage
BSOURCES     := $(wildcard src/*.cpp)
LSOURCES     := $(wildcard floret/*.cpp)
PGRAM        := $(wildcard peg/*.peg)
PSOURCES     := $(PGRAM:.peg=.cpp)
LOBJECTS     := $(LSOURCES:.cpp=.o)
BOBJECTS     := $(BSOURCES:.cpp=.o)
LDEPS        := $(LSOURCES:.cpp=.d)
BDEPS        := $(BSOURCES:.cpp=.d)
LTARGET      = libfloret.a
BTARGET      = floret-linux

all: deps grammar lib $(BTARGET)

deps: gdstk pgen ruler

gdstk:
	$(MAKE) -s $(MAKE_FLAGS) -C deps/gdstk lib

pgen:
	$(MAKE) -s $(MAKE_FLAGS) -C deps/pgen

ruler:
	$(MAKE) -s $(MAKE_FLAGS) -C deps/ruler

grammar: $(PSOURCES)

peg/%.cpp: peg/%.peg
	deps/pgen/pgen-linux $<
	mv peg/*.cpp peg/*.h floret
	
lib: $(LTARGET)

test: lib $(BTARGET) $(TTARGET)

check: test
	./$(TTARGET)

$(LTARGET): $(LOBJECTS)
	ar rvs $(LTARGET) $(LOBJECTS)

$(BTARGET): $(BOBJECTS) $(LTARGET)
	$(CXX) $(CXXFLAGS) $(BOBJECTS) -l:$(LTARGET) -l:libruler.a -l:libpgen.a -l:libgdstk.a -l:libclipper.a -l:libqhullstatic_r.a -lz -o $(BTARGET)

floret/%.o: floret/%.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

test/gtest_main.o: $(GTEST)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

-include $(LDEPS)
-include $(BDEPS)
-include $(TDEPS)

clean:
	rm -f src/*.o floret/*.o
	rm -f src/*.d floret/*.d
	rm -f $(LTARGET) $(BTARGET)

.PHONY: deps

PYTHON_RELEASE = python$(shell python3 -c "import sys;sys.stdout.write('{}.{}'.format(sys.version_info[0],sys.version_info[1]))")
CXXFLAGS     = -std=c++17 -g -O2 -Wall -fmessage-length=0 -I. -L. -Ideps/gdstk/include -Ideps/phy -Ideps/interpret_phy -Ideps/interpret_sch -Ideps/interpret_rect -Ideps/parse_spice -Ideps/parse_act -Ideps/parse -Ideps/common -Ideps/sch -Ldeps/gdstk/build/install/lib -Ldeps/gdstk/build/install/lib64  -Ldeps/phy -Ldeps/interpret_phy -Ldeps/interpret_sch -Ldeps/interpret_rect -Ldeps/parse_spice -Ldeps/parse_act -Ldeps/parse -Ldeps/common -Ldeps/sch
# -g -fprofile-arcs -ftest-coverage
BSOURCES     := $(wildcard src/*.cpp)
PGRAM        := $(wildcard peg/*.peg)
PSOURCES     := $(PGRAM:peg/%.peg=src/%.cpp)
BOBJECTS     := $(BSOURCES:.cpp=.o)
BDEPS        := $(BSOURCES:.cpp=.d)
BTARGET      = floret-linux

all: deps $(BTARGET)

deps:
	$(MAKE) -s $(MAKE_FLAGS) -C deps/gdstk lib
	$(MAKE) -s $(MAKE_FLAGS) -C deps/common
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse_spice
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse_act
	$(MAKE) -s $(MAKE_FLAGS) -C deps/phy
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_phy
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_sch
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_rect
	$(MAKE) -s $(MAKE_FLAGS) -C deps/sch

test: $(BTARGET) $(TTARGET)

check: test
	./$(TTARGET)

$(BTARGET): $(BOBJECTS)
	$(CXX) $(CXXFLAGS) $(BOBJECTS) -l:libsch.a -l:libphy.a -l:libinterpret_phy.a -l:libinterpret_sch.a -l:libinterpret_rect.a -l$(PYTHON_RELEASE) -l:libparse_spice.a -l:libparse_act.a -l:libparse.a -l:libcommon.a -l:libgdstk.a -l:libclipper.a -l:libqhullstatic_r.a -lz -o $(BTARGET)

floret/%.o: floret/%.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

test/gtest_main.o: $(GTEST)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

-include $(BDEPS)
-include $(TDEPS)

clean:
	$(MAKE) -s $(MAKE_FLAGS) -C deps/gdstk clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/common clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse_spice clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/parse_act clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/phy clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_phy clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_sch clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_rect clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/sch clean
	rm -f src/*.o
	rm -f src/*.d
	rm -f $(BTARGET)

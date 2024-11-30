.PHONY: deps

PYTHON_RELEASE = python$(shell python3 -c "import sys;sys.stdout.write('{}.{}'.format(sys.version_info[0],sys.version_info[1]))")

NAME          = floret
DEPEND        = phy sch interpret_phy interpret_sch interpret_rect parse_spice parse_act parse common

SRCDIR        = src

INCLUDE_PATHS = $(DEPEND:%=-Ideps/%) -Ideps/gdstk/build/install/include $(shell python3-config --includes) -I.
LIBRARY_PATHS = $(DEPEND:%=-Ldeps/%) -L$(shell python3-config --prefix)/lib -L.
LIBRARIES     = $(DEPEND:%=-l%) -l$(PYTHON_RELEASE)
LIBFILES      = $(foreach dep,$(DEPEND),deps/$(dep)/lib$(dep).a)
CXXFLAGS      = -std=c++17 -O2 -g -Wall -fmessage-length=0
LDFLAGS	      =  

SOURCES	     := $(shell mkdir -p $(SRCDIR); find $(SRCDIR) -name '*.cpp')
OBJECTS	     := $(SOURCES:%.cpp=build/%.o)
DEPS         := $(shell mkdir -p build/$(SRCDIR); find build/$(SRCDIR) -name '*.d')
TARGET	      = $(NAME)

ifeq ($(OS),Windows_NT)
	CXXFLAGS += -D WIN32
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		CXXFLAGS += -D AMD64
	else
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CXXFLAGS += -D AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CXXFLAGS += -D IA32
		endif
	endif
	LIBRARIES += -l:libgdstk.a -l:libclipper.a -l:libqhullstatic_r.a -lz
	LIBRARY_PATHS += -Ldeps/gdstk/build/install/lib -Ldeps/gdstk/build/install/lib64
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CXXFLAGS += -D LINUX
		LIBRARIES += -l:libgdstk.a -l:libclipper.a -l:libqhullstatic_r.a -lz
		LIBRARY_PATHS += -Ldeps/gdstk/build/install/lib -Ldeps/gdstk/build/install/lib64
	endif
	ifeq ($(UNAME_S),Darwin)
		CXXFLAGS += -D OSX -mmacos-version-min=12.0 -Wno-missing-braces
		INCLUDE_PATHS += -I$(shell brew --prefix qhull)/include -I$(shell brew --prefix graphviz)/include
		LIBRARY_PATHS += -L$(shell brew --prefix qhull)/lib -L$(shell brew --prefix graphviz)/lib
		LIBRARIES += -lgdstk -lclipper -lqhullstatic_r -lz
		LIBRARY_PATHS += -Ldeps/gdstk/build/install/lib
	endif
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		CXXFLAGS += -D AMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		CXXFLAGS += -D IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		CXXFLAGS += -D ARM
	endif
endif

all: deps $(TARGET)

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

$(TARGET): $(OBJECTS) $(LIBFILES)
	$(CXX) $(LIBRARY_PATHS) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/$(SRCDIR)/%.o: $(SRCDIR)/%.cpp 
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

include $(DEPS) $(TEST_DEPS)

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
	rm -f $(TARGET)

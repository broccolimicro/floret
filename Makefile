.PHONY: deps

NAME          = floret
DEPEND        = phy sch interpret_phy interpret_sch parse_spice parse_act parse common

SRCDIR        = src

INCLUDE_PATHS = $(DEPEND:%=-Ideps/%) -Ideps/gdstk/build/include $(shell python3-config --includes) -I.
LIBRARY_PATHS = $(DEPEND:%=-Ldeps/%) -L.
LIBRARIES     = $(DEPEND:%=-l%) -ldl
LIBFILES      = $(foreach dep,$(DEPEND),deps/$(dep)/lib$(dep).a)
CXXFLAGS      = -std=c++17 -O2 -g -Wall -fmessage-length=0 -D CL_HPP_MINIMUM_OPENCL_VERSION=120 -D CL_HPP_TARGET_OPENCL_VERSION=120 -D CL_HPP_ENABLE_EXCEPTIONS 

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
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CXXFLAGS += -D IA32
		else # ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CXXFLAGS += -D AMD64
		endif
	endif
	LIBRARIES += -l:libgdstk.a -l:libqhullstatic_r.a -lz -lOpenCL
	LIBRARY_PATHS += -Ldeps/gdstk/build/lib -Ldeps/gdstk/build/lib64
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		CXXFLAGS += -D OSX -mmacos-version-min=26.0 -Wno-missing-braces
		INCLUDE_PATHS += -I$(shell brew --prefix qhull)/include -I$(shell brew --prefix graphviz)/include -I$(shell brew --prefix opencl-headers)/include -I$(shell brew --prefix opencl-clhpp-headers)/include
		LIBRARY_PATHS += -L$(shell brew --prefix qhull)/lib -L$(shell brew --prefix graphviz)/lib
		LIBRARIES += -lgdstk -lqhullstatic_r -lz -framework OpenCL
		LIBRARY_PATHS += -Ldeps/gdstk/build/lib
		LDFLAGS	      += -Wl,-rpath,/opt/homebrew/opt/python@3.15/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.12/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.11/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.10/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3.09/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python@3/Frameworks/Python.framework/Versions/Current/lib \
-Wl,-rpath,/opt/homebrew/opt/python/Frameworks/Python.framework/Versions/Current/lib
	else # ifeq ($(UNAME_S),Linux)
		CXXFLAGS += -D LINUX
		LIBRARIES += -l:libgdstk.a -l:libqhullstatic_r.a -lz -lOpenCL
		LIBRARY_PATHS += -Ldeps/gdstk/build/lib -Ldeps/gdstk/build/lib64
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
	#$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_rect
	$(MAKE) -s $(MAKE_FLAGS) -C deps/sch

$(TARGET): $(OBJECTS) $(LIBFILES) deps
	$(CXX) $(LIBRARY_PATHS) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/$(SRCDIR)/%.o: $(SRCDIR)/%.cpp 
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

linux: $(TARGET)
	mkdir -p floret-linux/usr/local/bin
	cp floret floret-linux/usr/local/bin
	mkdir -p floret-linux/usr/local/share/
	cp -r tech floret-linux/usr/local/share
	mkdir -p floret-linux/DEBIAN
	mkdir -p debian
	touch debian/control
	echo "Package: floret" > floret-linux/DEBIAN/control
	cat VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\)/\1.\2-\3/g' | xargs -I{} echo "Version: {}" >> floret-linux/DEBIAN/control
	echo "Section: base" >> floret-linux/DEBIAN/control
	echo "Priority: optional" >> floret-linux/DEBIAN/control
	echo "Architecture: amd64" >> floret-linux/DEBIAN/control
	dpkg-shlibdeps -O floret-linux/usr/local/bin/floret | sed 's/.*Depends=/Depends: /g' >> floret-linux/DEBIAN/control
	echo "Maintainer: $(MAINTAINER_NAME) <$(MAINTAINER_EMAIL)>" >> floret-linux/DEBIAN/control
	echo "Description: Loom" >> floret-linux/DEBIAN/control
	echo " A programming language for quasi-delay insensitive asynchronous circuits" >> floret-linux/DEBIAN/control
	dpkg-deb --build --root-owner-group floret-linux
	rm -rf debian floret-linux

windows: $(TARGET)
	mkdir -p Floret/bin
	mkdir -p Floret/share
	cp floret.exe Floret/bin/floret.exe
	cp -r tech Floret/share/tech
	ldd Floret/bin/floret.exe | grep "mingw64" | sed 's/.*\/mingw64/\/mingw64/g' | sed 's/ (.*$$//g' | xargs -I{} cp {} Floret/bin
	zip -r floret-windows.zip Floret
	rm -rf Floret

macos: $(TARGET)
	mkdir -p floret-macos/bin
	mkdir -p floret-macos/share
	cp floret floret-macos/bin/floret
	cp -r tech floret-macos/share/tech
	tar -czvf floret-macos.tar.gz floret-macos


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
	#$(MAKE) -s $(MAKE_FLAGS) -C deps/interpret_rect clean
	$(MAKE) -s $(MAKE_FLAGS) -C deps/sch clean
	rm -f src/*.o
	rm -f src/*.d
	rm -f $(TARGET)

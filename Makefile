CXXFLAGS	 =  -g -O2 -Wall -fmessage-length=0
INCLUDE_DIRS = -Ideps/pgen
LIBRARY_DIRS = -Ldeps/pgen/build
LIBRARIES = -l:libpgen.a

GRAMMAR := $(wildcard peg/*.peg)
GRAMSRC := $(GRAMMAR:peg/%.peg=src/%.cpp)
GRAMHDR := $(GRAMMAR:peg/%.peg=src/%.h)
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=obj/%.o)
DEPS := $(OBJECTS:.o=.d)
TARGET = floret

-include $(DEPS)

all: $(TARGET)

grammar: $(GRAMSRC)

src/%.cpp: peg/%.peg
	deps/pgen/build/pgen-linux $<
	mv peg/*.cpp peg/*.h src/

$(TARGET): $(OBJECTS)
	g++ $(CXXFLAGS) $(LIBRARY_DIRS) -o $(TARGET) $(OBJECTS) $(LIBRARIES)

obj/%.o: src/%.cpp obj
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c -o $@ $<

obj:
	@mkdir -p obj

clean:
	rm -rf obj
	rm -f $(TARGET)

clobber: clean
	rm -f $(GRAMSRC) $(GRAMHDR)

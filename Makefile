CXXFLAGS	 =  -g -O2 -Wall -fmessage-length=0
INCLUDE_DIRS =
LIBRARY_DIRS =
LIBRARIES =

GRAMMAR := $(wildcard src/*.peg)
GRAMSRC := $(GRAMMAR:src/%.peg=src/%.cpp)
GRAMHDR := $(GRAMMAR:src/%.peg=src/%.h)
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=obj/%.o)
DEPS := $(OBJECTS:.o=.d)
TARGET = floret

-include $(DEPS)

all: $(TARGET)

grammar: $(GRAMSRC)

src/%.cpp: src/%.peg
	pgen $<

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

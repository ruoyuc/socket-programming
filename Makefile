GXX = g++
OBJECTS = client serverM serverC serverCS serverEE
FLAGS = -std=c++11
JUNKS = *.out $(OBJECTS)

all: $(OBJECTS)

%: %.cpp
	$(GXX) $(FLAGS) $^ -o $@

clean:
	rm -rf $(JUNKS)



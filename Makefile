UNAME_S := $(shell uname -s)
CXXFLAGS = -std=c++2a -Wall -pedantic -Wformat -O3 -lraylib

ifeq ($(UNAME_S),Linux)
	LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
ifeq ($(UNAME_S),Darwin)
	LIBS = -lm -lpthread -ldl -framework IOKit -framework Cocoa -framework OpenGL `pkg-config --libs --cflags raylib`
endif

MAINSRC=$(wildcard src/main.cpp)
OBJ=$(addsuffix .o,$(basename $(MAINSRC)))

.PHONY: all debug

all: main

debug: CXXFLAGS += -g3 -O0
debug: main

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f ./src/*.o
	rm -f ./main
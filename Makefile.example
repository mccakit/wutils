CXX ?= g++
CXXSTD ?= c++23
CXXFLAGS = -Wall -Wextra -Werror -march=native -std=$(CXXSTD) -O0 -g

SRCS = wutils.cpp testwutils.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

TARGET = testwutils

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^

# Phony targets
clean:
	rm $(TARGET) $(OBJS)

.PHONY: all clean

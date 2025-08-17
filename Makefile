CXX ?= g++
CXXSTD ?= c++23
CXXFLAGS = -Wall -Wextra -Werror -march=native -std=$(CXXSTD)

SRCS = wutils.cpp testwutils.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

TARGET = testwutils

BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),release)
	CXXFLAGS += -O3
else ifeq ($(BUILD_TYPE),debug)
	CXXFLAGS += -O0 -g
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^

# Phony targets
release:
	$(MAKE) BUILD_TYPE=release

debug:
	$(MAKE) BUILD_TYPE=debug

clean:
	rm $(TARGET) $(OBJS)

.PHONY: all release debug clean

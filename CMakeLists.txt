# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")

# Project information
project(TestWutils CXX)

# Set the required C++ standard to C++23.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler-specific flags.
if(MSVC)
    # MSVC (nmake) specific flags
    set(CMAKE_CXX_FLAGS "/W4 /WX /EHsc /utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
elseif(UNIX)
    # GNU Make specific flags
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

# Define the source files
set(TEST_SOURCES wutils.cpp testwutils.cpp)
set(EXAMPLE_SOURCES wutils.cpp example.cpp)

# Add executable target
add_executable(testwutils ${TEST_SOURCES})
add_executable(example ${EXAMPLE_SOURCES})

# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project information
project(TestWutils CXX)

# Set the required C++ standard to C++23.
# This aligns with the 'c++23' and 'c++23preview' flags from your Makefiles.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler-specific flags. These mirror the flags from your original Makefiles.
# The 'CMAKE_CXX_FLAGS' variable is used for all build types (e.g., Debug, Release).
# The 'CMAKE_CXX_FLAGS_DEBUG' variable is for debug builds specifically.
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
set(SOURCES wutils.cpp testwutils.cpp)

# Add an executable target named 'testwutils' from the source files
add_executable(testwutils ${SOURCES})

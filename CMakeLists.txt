cmake_minimum_required(VERSION 4.2.2)
project(Wutils CXX)
add_library(wutils STATIC)
target_include_directories(wutils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
option(USE_WUTILS_MODULE "Enable WUtils module support" OFF)
if(USE_WUTILS_MODULE)
    target_sources(wutils
        PUBLIC
            FILE_SET cxx_modules
            TYPE CXX_MODULES
            FILES src/wutils.cppmm
    )
else()
    target_sources(wutils PRIVATE src/wutils.cpp)
endif()
if(NOT CMAKE_CROSSCOMPILING)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)
    add_executable(testwutils tests/testwutils.cpp)
    target_link_libraries(testwutils PRIVATE wutils PkgConfig::GTEST)
    enable_testing()
    add_test(NAME testwutils COMMAND testwutils)
endif()

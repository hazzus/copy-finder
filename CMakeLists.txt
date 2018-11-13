cmake_minimum_required(VERSION 3.0)

project(CopyFinder)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -O2 -pedantic")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

find_package(Qt5Widgets REQUIRED)

add_library(xxhash STATIC xxhash_cpp/xxhash/xxhash.h xxhash_cpp/xxhash/xxhash.cpp xxhash_cpp/xxhash/xxhash.hpp)

include_directories(xxhash_cpp/xxhash)

add_executable(CopyFinder main.cpp
    mainwindow.h mainwindow.cpp
    resultdialog.h resultdialog.cpp)

target_link_libraries(CopyFinder xxhash -lpthread)

set_target_properties(CopyFinder PROPERTIES
    CXX_STANDARD 17)

qt5_use_modules(CopyFinder Widgets)

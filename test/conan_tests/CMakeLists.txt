cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(a)

find_package(ppplugin REQUIRED)

message(DEBUG "ppplugin found: ${ppplugin_FOUND}")
message(DEBUG "ppplugin should be shared library: ${PPPLUGIN_SHARED}")
message(DEBUG "ppplugin library dir: ${PPPLUGIN_LIBRARY_DIR}")
message(DEBUG "ppplugin include dir: ${PPPLUGIN_INCLUDE_DIR}")
message(
  DEBUG
  "ppplugin should be C++17 compatible: ${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY}")

if(${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY})
  set(CMAKE_CXX_STANDARD 17)
else()
  set(CMAKE_CXX_STANDARD 20)
endif()

add_executable(a a.cpp)
target_link_libraries(a PUBLIC ppplugin::ppplugin)

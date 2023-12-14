cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(ppplugin)

option(PPPLUGIN_ENABLE_EXAMPLES "Enable compilation of examples" OFF)
option(PPPLUGIN_ENABLE_TESTS "Enable compilation of tests" OFF)
option(PPPLUGIN_ENABLE_COVERAGE "Enable compilation with test coverage flags"
       OFF)
option(PPPLUGIN_ENABLE_ADDRESS_SANITIZE
       "Enable compilation with address sanitize flags" OFF)
option(PPPLUGIN_ENABLE_THREAD_SANITIZE
       "Enable compilation with thread sanitize flags" OFF)
option(PPPLUGIN_ENABLE_UNDEFINED_SANITIZE
       "Enable compilation with undefined sanitize flags" OFF)
option(PPPLUGIN_ENABLE_UNREACHABLE_SANITIZE
       "Enable compilation with unreachable sanitize flags" OFF)
option(PPPLUGIN_ENABLE_CPP17_COMPATIBILITY
       "Enable compilation with unreachable sanitize flags" OFF)

find_package(Boost 1.68.0 REQUIRED COMPONENTS program_options filesystem)
# TODO: check if necessary
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# if not specified otherwise, build in release mode
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(LIBRARY_NAME "ppplugin")
set(LIBRARY_VERSION "0.1.0")

# enable generation of compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find includes in corresponding build directories TODO: check if necessary
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

set(OPTIMIZE_FLAGS "-O2")
set(DEBUG_FLAGS "-g3")

if(${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY})
  find_package(fmt 8.1.0 REQUIRED)
  set(CMAKE_CXX_STANDARD 17)
else()
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# TODO: check if necessary, especially -pthread
set(CMAKE_CXX_FLAGS
    "-ffast-math -Wall -Wextra -Wno-psabi -pedantic-errors -Werror \
     ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${OPTIMIZE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${DEBUG_FLAGS} ${OPTIMIZE_FLAGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

if(${PPPLUGIN_ENABLE_COVERAGE})
  set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} -lgcov -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -lgcov -fprofile-arcs -ftest-coverage")
endif()

if(${PPPLUGIN_ENABLE_ADDRESS_SANITIZE})
  set(CMAKE_CXX_FLAGS "-fsanitize=address ${CMAKE_CXX_FLAGS}")
endif()
if(${PPPLUGIN_ENABLE_THREAD_SANITIZE})
  set(CMAKE_CXX_FLAGS "-fsanitize=thread ${CMAKE_CXX_FLAGS}")
endif()
if(${PPPLUGIN_ENABLE_UNDEFINED_SANITIZE})
  set(CMAKE_CXX_FLAGS "-fsanitize=undefined ${CMAKE_CXX_FLAGS}")
endif()
if(${PPPLUGIN_ENABLE_UNREACHABLE_SANITIZE})
  set(CMAKE_CXX_FLAGS "-fsanitize=unreachable ${CMAKE_CXX_FLAGS}")
endif()

message(STATUS "Compiling in ${CMAKE_BUILD_TYPE} mode")

add_subdirectory(src)

if(${PPPLUGIN_ENABLE_EXAMPLES})
  add_subdirectory(examples)
endif()
if(${PPPLUGIN_ENABLE_TESTS})
  add_subdirectory(test)
endif()

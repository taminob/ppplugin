cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(
  ppplugin
  VERSION 0.0.0
  DESCRIPTION "Library for uniform C++ interface to multi-language plugins"
  LANGUAGES C CXX)

option(PPPLUGIN_SHARED "Compile library as shared library" OFF)
option(PPPLUGIN_ENABLE_EXAMPLES "Enable compilation of examples" OFF)
option(PPPLUGIN_ENABLE_CPP17_COMPATIBILITY
       "Enable compilation with unreachable sanitize flags" OFF)
option(PPPLUGIN_ENABLE_LUA_PLUGINS "Enable compilation with Lua plugin support"
       ON)
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

find_package(Boost 1.70.0 REQUIRED CONFIG COMPONENTS headers filesystem)
find_package(Python 3.0 REQUIRED COMPONENTS Development)
find_package(Lua 5.2 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# if not specified otherwise, build in release mode
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(${PPPLUGIN_SHARED})
  set(BUILD_SHARED_LIBS ON)
endif()

set(LIBRARY_NAME "ppplugin")
set(LIBRARY_TARGET ${LIBRARY_NAME})
if(${BUILD_SHARED_LIBS})
  set(LIBRARY_TYPE "shared")
else()
  set(LIBRARY_TYPE "static")
endif()

# enable generation of compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find includes in source and build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

set(CMAKE_CXX_FLAGS
    "-ffast-math -Wall -Wextra -Wpedantic -pedantic-errors -Werror \
     ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${OPTIMIZE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${DEBUG_FLAGS} ${OPTIMIZE_FLAGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

if(${PPPLUGIN_ENABLE_COVERAGE})
  set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_C_FLAGS} ")
  set(CMAKE_SHARED_LINKER_FLAGS
      "-lgcov -fprofile-arcs -ftest-coverage ${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS
      "-lgcov -fprofile-arcs -ftest-coverage ${CMAKE_EXE_LINKER_FLAGS}")
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

# generate and install cmake files for find_package
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(CONFIG_CMAKE_FILENAME "${PROJECT_NAME}-config.cmake")
set(TARGETS_SHARED_CMAKE_FILENAME "${PROJECT_NAME}-shared-targets.cmake")
set(TARGETS_STATIC_CMAKE_FILENAME "${PROJECT_NAME}-static-targets.cmake")
set(TARGETS_CMAKE_FILENAME "${PROJECT_NAME}-${LIBRARY_TYPE}-targets.cmake")
set(VERSION_CMAKE_FILENAME "${PROJECT_NAME}-version.cmake")
set(GENERATED_CMAKE_FILES_OUTPUT_DIR "${CMAKE_BINARY_DIR}/cmake/")
set(INSTALLED_CMAKE_FILES_DESTINATION_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
configure_package_config_file(
  "cmake/${CONFIG_CMAKE_FILENAME}.in"
  "${GENERATED_CMAKE_FILES_OUTPUT_DIR}/${CONFIG_CMAKE_FILENAME}"
  INSTALL_DESTINATION "${INSTALLED_CMAKE_FILES_DESTINATION_DIR}")
write_basic_package_version_file(
  "${GENERATED_CMAKE_FILES_OUTPUT_DIR}/${VERSION_CMAKE_FILENAME}"
  VERSION ${ppplugin_VERSION}
  COMPATIBILITY SameMajorVersion)
install(FILES "${GENERATED_CMAKE_FILES_OUTPUT_DIR}/${CONFIG_CMAKE_FILENAME}"
              "${GENERATED_CMAKE_FILES_OUTPUT_DIR}/${VERSION_CMAKE_FILENAME}"
        DESTINATION "${INSTALLED_CMAKE_FILES_DESTINATION_DIR}")

message(
  STATUS "Compiling in ${CMAKE_BUILD_TYPE} mode as ${LIBRARY_TYPE} library")

add_subdirectory(src)
if(${PPPLUGIN_ENABLE_EXAMPLES})
  add_subdirectory(examples)
endif()
if(${PPPLUGIN_ENABLE_TESTS})
  enable_testing()
  add_subdirectory(test)
endif()

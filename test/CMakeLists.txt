find_package(GTest REQUIRED)

set(TESTS_NAME "tests")

add_executable(${TESTS_NAME} detail_templates_tests.cpp expected_tests.cpp
                             scope_guard_tests.cpp)
target_include_directories(${TESTS_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TESTS_NAME} PRIVATE GTest::GTest GTest::Main gmock
                                            Threads::Threads ${LIBRARY_TARGET})

add_subdirectory(lua_tests)
add_subdirectory(python_tests)

gtest_discover_tests(${TESTS_NAME})

add_test(
  NAME cmake_shared_cpp17_compatible_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_shared_cpp17 "SHARED" "Cpp17"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)
add_test(
  NAME cmake_shared_cpp20_compatible_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_shared_cpp20 "SHARED" "Cpp20"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)
add_test(
  NAME cmake_static_cpp17_compatible_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_static_cpp17 "STATIC" "Cpp17"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)
add_test(
  NAME cmake_static_cpp20_compatible_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_static_cpp20 "STATIC" "Cpp20"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)

add_test(
  NAME cmake_no_shared_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_shared_fail "SHARED_FAIL"
    "Cpp20"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)
set_property(TEST cmake_no_shared_installation_test PROPERTY WILL_FAIL TRUE)
add_test(
  NAME cmake_no_static_installation_test
  COMMAND
    test.sh ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_find_package_test_build
    ${CMAKE_CURRENT_BINARY_DIR}/test_install_root_static_fail "STATIC_FAIL"
    "Cpp20"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_tests/find_package_test)
set_property(TEST cmake_no_static_installation_test PROPERTY WILL_FAIL TRUE)

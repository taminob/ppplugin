find_package(GTest REQUIRED)

set(TESTS_NAME "tests")

add_executable(${TESTS_NAME} detail_templates_tests.cpp expected_tests.cpp
                             scope_guard_tests.cpp)

target_include_directories(${TESTS_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
  ${TESTS_NAME} PRIVATE GTest::GTest GTest::Main gmock Threads::Threads
                        ${LIBRARY_TARGET_STATIC})

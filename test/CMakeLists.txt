find_package(GTest REQUIRED)

set(TESTS_NAME "tests")

add_executable(${TESTS_NAME} detail_templates_tests.cpp)

target_include_directories(${TESTS_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
  ${TESTS_NAME}
  PRIVATE GTest::GTest
          GTest::Main
          gmock
          Boost::program_options
          Threads::Threads
          ${LIBRARY_NAME}_static)

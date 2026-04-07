target_sources(${TESTS_NAME} PRIVATE shell_tests.cpp)

add_custom_target(shell_tests ALL COMMENT "Shell test files")
add_custom_command(
  TARGET shell_tests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
          ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  COMMENT "Copying POSIX shell plugins to output directory...")

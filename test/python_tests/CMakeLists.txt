target_sources(${TESTS_NAME} PRIVATE python_tests.cpp)

add_custom_target(python_tests ALL)
add_custom_command(
  TARGET python_tests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.py
          ${CMAKE_CURRENT_BINARY_DIR}/test.py
  COMMENT "Copying Python plugins to output directory...")

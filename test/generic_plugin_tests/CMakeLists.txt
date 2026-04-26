target_sources(${TESTS_NAME} PRIVATE generic_plugin_tests.cpp)

add_custom_target(generic_plugin_tests ALL COMMENT "GenericPlugin test files")
add_custom_command(
  TARGET generic_plugin_tests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.py
          ${CMAKE_CURRENT_BINARY_DIR}/test.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.lua
          ${CMAKE_CURRENT_BINARY_DIR}/test.lua
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
          ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  COMMENT "Copying generic test plugins to output directory...")

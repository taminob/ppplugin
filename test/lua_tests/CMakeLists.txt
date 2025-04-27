target_sources(${TESTS_NAME} PRIVATE lua_tests.cpp)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.lua
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.lua
          ${CMAKE_CURRENT_BINARY_DIR}/test.lua
  COMMENT "Copying Lua plugins to output directory...")

add_executable(lua_plugin_manager lua_plugin_manager.cpp)
target_link_libraries(lua_plugin_manager ${LIBRARY_TARGET})

add_custom_command(
  TARGET lua_plugin_manager
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.lua
          $<TARGET_FILE_DIR:lua_plugin_manager>/plugin_1.lua
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.lua
          $<TARGET_FILE_DIR:lua_plugin_manager>/plugin_2.lua)

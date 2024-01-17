add_executable(multi_return_lua_plugin_manager
               multi_return_lua_plugin_manager.cpp)
target_include_directories(multi_return_lua_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(multi_return_lua_plugin_manager ${LIBRARY_TARGET_SHARED})

add_custom_command(
  TARGET multi_return_lua_plugin_manager
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.lua
          $<TARGET_FILE_DIR:multi_return_lua_plugin_manager>)

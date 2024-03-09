add_executable(multi_language_plugin_manager multi_language_plugin_manager.cpp)
target_include_directories(multi_language_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(multi_language_plugin_manager ${LIBRARY_TARGET_SHARED})

add_library(cpp_plugin SHARED plugins/cpp_plugin.cpp)
add_library(c_plugin SHARED plugins/c_plugin.c)

add_custom_command(
  TARGET multi_language_plugin_manager
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugins/lua_plugin.lua
    $<TARGET_FILE_DIR:multi_language_plugin_manager>)
add_custom_command(
  TARGET multi_language_plugin_manager
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugins/python_plugin.py
    $<TARGET_FILE_DIR:multi_language_plugin_manager>)

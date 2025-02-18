add_executable(python_plugin_manager python_plugin_manager.cpp)
target_link_libraries(python_plugin_manager PRIVATE ${LIBRARY_TARGET})

add_custom_command(
  TARGET python_plugin_manager
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.py
          $<TARGET_FILE_DIR:python_plugin_manager>/plugin_1.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.py
          $<TARGET_FILE_DIR:python_plugin_manager>/plugin_2.py
  COMMENT "Copying Python plugins to output directory...")

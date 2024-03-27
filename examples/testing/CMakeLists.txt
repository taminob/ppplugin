add_executable(testing testing.cpp)
target_link_libraries(testing ${LIBRARY_TARGET})

add_custom_command(
  TARGET testing
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.py
          $<TARGET_FILE_DIR:testing>/plugin_1.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/plugin.py
          $<TARGET_FILE_DIR:testing>/plugin_2.py)

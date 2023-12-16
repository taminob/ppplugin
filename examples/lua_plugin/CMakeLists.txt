add_executable(lua_plugin_manager lua_plugin_manager.cpp)
target_include_directories(lua_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lua_plugin_manager ${LIBRARY_TARGET_SHARED}
                      Boost::filesystem lua)

add_library(lua_plugin SHARED lua_plugin.cpp)
target_include_directories(lua_plugin PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lua_plugin ${LIBRARY_TARGET_SHARED} Boost::filesystem)

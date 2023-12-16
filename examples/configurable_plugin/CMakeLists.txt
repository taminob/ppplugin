add_executable(configurable_plugin_manager configurable_plugin_manager.cpp)
target_include_directories(configurable_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(configurable_plugin_manager ${LIBRARY_TARGET_SHARED}
                      Boost::filesystem)

add_library(configurable_plugin SHARED configurable_plugin.cpp)
target_include_directories(configurable_plugin
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(configurable_plugin ${LIBRARY_TARGET_SHARED}
                      Boost::filesystem)

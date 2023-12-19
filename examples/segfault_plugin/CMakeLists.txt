add_executable(segfault_plugin_manager segfault_plugin_manager.cpp)
target_include_directories(segfault_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(segfault_plugin_manager ${LIBRARY_TARGET_SHARED}
                      Boost::filesystem)

add_library(valid_plugin SHARED plugins/valid_plugin.cpp)
add_library(segfault_plugin SHARED plugins/segfault_plugin.cpp)

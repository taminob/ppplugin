add_executable(simple_plugin_manager simple_plugin_manager.cpp)
target_link_libraries(simple_plugin_manager ${LIBRARY_TARGET})

add_library(simple_plugin SHARED simple_plugin.cpp)

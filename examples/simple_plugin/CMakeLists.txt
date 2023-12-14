add_executable(simple_plugin_manager simple_plugin_manager.cpp)
target_include_directories(simple_plugin_manager
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(simple_plugin_manager ${LIBRARY_NAME}_static
                      Boost::filesystem)

add_library(simple_plugin SHARED simple_plugin.cpp)
target_include_directories(simple_plugin PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(simple_plugin ${LIBRARY_NAME}_static Boost::filesystem)

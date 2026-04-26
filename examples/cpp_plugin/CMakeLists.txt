add_executable(cpp_example_1 example_1.cpp)
target_link_libraries(cpp_example_1 ${LIBRARY_TARGET})

add_executable(cpp_example_2 example_2.cpp)
target_link_libraries(cpp_example_2 ${LIBRARY_TARGET})

add_executable(cpp_example_3 example_3.cpp)
target_link_libraries(cpp_example_3 ${LIBRARY_TARGET})

add_library(cpp_plugin SHARED cpp_plugin.cpp)

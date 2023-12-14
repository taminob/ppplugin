set(LIBRARY_SOURCES ppplugin.cpp)

add_library(${LIBRARY_NAME}_object OBJECT ${LIBRARY_SOURCES})
set_property(TARGET ${LIBRARY_NAME}_object PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(${LIBRARY_NAME}_object PUBLIC Boost::program_options
                                                    Boost::filesystem)

target_include_directories(${LIBRARY_NAME}_object
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(${LIBRARY_NAME}_shared SHARED
            $<TARGET_OBJECTS:${LIBRARY_NAME}_object>)
add_library(${LIBRARY_NAME}_static STATIC
            $<TARGET_OBJECTS:${LIBRARY_NAME}_object>)

set_target_properties(${LIBRARY_NAME}_shared PROPERTIES OUTPUT_NAME
                                                        ${LIBRARY_NAME})
set_target_properties(${LIBRARY_NAME}_static PROPERTIES OUTPUT_NAME
                                                        ${LIBRARY_NAME})

install(
  TARGETS ${LIBRARY_NAME}_object
  INCLUDES
  DESTINATION include)
install(TARGETS ${LIBRARY_NAME}_shared LIBRARY DESTINATION lib)
install(TARGETS ${LIBRARY_NAME}_static LIBRARY DESTINATION lib)

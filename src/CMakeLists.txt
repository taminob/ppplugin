set(LIBRARY_SOURCES ppplugin.cpp)

set(LIBRARY_PUBLIC_HEADERS "plugin.h" "plugin_manager.h")
set(LIBRARY_SOURCES "ppplugin.cpp" "boost_dll_loader.cpp" "lua_state.cpp" "lua_script.cpp")

set(LIBRARY_TARGET_OBJECT "${LIBRARY_NAME}_object")

add_library(${LIBRARY_TARGET_OBJECT} OBJECT ${LIBRARY_SOURCES})
set_target_properties(
  ${LIBRARY_TARGET_OBJECT} PROPERTIES PUBLIC_HEADER "${LIBRARY_PUBLIC_HEADERS}"
                                      POSITION_INDEPENDENT_CODE 1)
target_link_libraries(${LIBRARY_NAME}_object PUBLIC Boost::filesystem lua)

target_include_directories(${LIBRARY_TARGET_OBJECT}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(${LIBRARY_TARGET_SHARED} SHARED
            $<TARGET_OBJECTS:${LIBRARY_TARGET_OBJECT}>)
add_library(${LIBRARY_TARGET_STATIC} STATIC
            $<TARGET_OBJECTS:${LIBRARY_TARGET_OBJECT}>)
target_link_libraries(
  ${LIBRARY_TARGET_SHARED}
  PUBLIC $<TARGET_PROPERTY:${LIBRARY_TARGET_OBJECT},LINK_LIBRARIES>)
target_link_libraries(
  ${LIBRARY_TARGET_STATIC}
  PUBLIC $<TARGET_PROPERTY:${LIBRARY_TARGET_OBJECT},LINK_LIBRARIES>)

set_target_properties(${LIBRARY_TARGET_SHARED} PROPERTIES OUTPUT_NAME
                                                          ${LIBRARY_NAME})
set_target_properties(${LIBRARY_TARGET_STATIC} PROPERTIES OUTPUT_NAME
                                                          ${LIBRARY_NAME})

install(
  TARGETS ${LIBRARY_TARGET_OBJECT}
  INCLUDES
  DESTINATION include/${LIBRARY_NAME})
install(TARGETS ${LIBRARY_TARGET_SHARED} LIBRARY DESTINATION lib)
install(TARGETS ${LIBRARY_TARGET_STATIC} LIBRARY DESTINATION lib)

set(LIBRARY_SOURCES ppplugin.cpp)

set(LIBRARY_SOURCES "ppplugin.cpp" "boost_dll_loader.cpp" "lua_state.cpp"
                    "lua_script.cpp" "python_plugin.cpp")

set(LIBRARY_TARGET_OBJECT "${LIBRARY_NAME}_object")

add_library(${LIBRARY_TARGET_OBJECT} OBJECT ${LIBRARY_SOURCES})
set_target_properties(${LIBRARY_TARGET_OBJECT}
                      PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_link_libraries(${LIBRARY_NAME}_object
                      PUBLIC Boost::filesystem Boost::python Python::Python lua)

target_include_directories(
  ${LIBRARY_TARGET_OBJECT}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

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

set_target_properties(
  ${LIBRARY_TARGET_SHARED}
  PROPERTIES OUTPUT_NAME ${LIBRARY_NAME}
             VERSION ${ppplugin_VERSION}
             SOVERSION ${ppplugin_VERSION_MAJOR}
             )
set_target_properties(
  ${LIBRARY_TARGET_STATIC} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME}
                                      VERSION ${PROJECT_VERSION})

get_filename_component(TARGETS_CMAKE_FILENAME_WITHOUT_EXTENSION
                       "${TARGETS_CMAKE_FILENAME}" NAME_WLE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
        DESTINATION "include/${LIBRARY_NAME}")
install(
  TARGETS ${LIBRARY_TARGET_SHARED} ${LIBRARY_TARGET_STATIC}
          ${LIBRARY_TARGET_OBJECT}
  EXPORT "${TARGETS_CMAKE_FILENAME_WITHOUT_EXTENSION}"
  LIBRARY DESTINATION lib)
# generate and install target files for cmake find_package
install(
  EXPORT "${TARGETS_CMAKE_FILENAME_WITHOUT_EXTENSION}"
  FILE "${TARGETS_CMAKE_FILENAME}"
  DESTINATION "${INSTALLED_CMAKE_FILES_DESTINATION_DIR}"
  NAMESPACE "${PROJECT_NAME}::")

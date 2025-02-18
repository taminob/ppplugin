set(LIBRARY_SOURCES ppplugin.cpp)

set(LIBRARY_SOURCES
    "ppplugin.cpp"
    "boost_dll_loader.cpp"
    "lua_state.cpp"
    "lua_script.cpp"
    "python_plugin.cpp"
    "python_interpreter.cpp"
    "python_exception.cpp"
    "python_tuple.cpp"
    "python_object.cpp"
    "python_guard.cpp")

add_library(${LIBRARY_TARGET} ${LIBRARY_SOURCES})
target_link_libraries(
  ${LIBRARY_TARGET}
  PRIVATE ${LUA_LIBRARIES}
  PUBLIC Boost::filesystem Python::Python
         $<$<BOOL:${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY}>:fmt::fmt>)
target_include_directories(
  ${LIBRARY_TARGET}
  PRIVATE ${LUA_INCLUDE_DIR}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_definitions(
  ${LIBRARY_TARGET}
  PUBLIC
    $<$<BOOL:${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY}>:"PPPLUGIN_CPP17_COMPATIBILITY">
)

set_target_properties(
  ${LIBRARY_TARGET}
  PROPERTIES OUTPUT_NAME ${LIBRARY_NAME}
             VERSION ${ppplugin_VERSION}
             SOVERSION ${ppplugin_VERSION_MAJOR})

set(TARGETS_EXPORT_NAME "${PROJECT_NAME}-targets")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
        DESTINATION "include/${LIBRARY_NAME}")
install(
  TARGETS ${LIBRARY_TARGET}
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION lib)
# generate and install target files for cmake find_package
install(
  EXPORT "${TARGETS_EXPORT_NAME}"
  FILE "${TARGETS_CMAKE_FILENAME}"
  DESTINATION "${INSTALLED_CMAKE_FILES_DESTINATION_DIR}"
  NAMESPACE "${PROJECT_NAME}::")

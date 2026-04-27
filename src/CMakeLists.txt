set(LIBRARY_SOURCES
    "${CMAKE_SOURCE_DIR}/include/ppplugin.cpp"
    "boost_dll_loader.cpp"
    "c/plugin.cpp"
    "cpp/plugin.cpp"
    "lua/plugin.cpp"
    "lua/lua_state.cpp"
    "lua/lua_script.cpp"
    "python/plugin.cpp"
    "python/python_interpreter.cpp"
    "python/python_exception.cpp"
    "python/python_tuple.cpp"
    "python/python_object.cpp"
    "python/python_guard.cpp"
    "shell/plugin.cpp"
    "shell/shell_session.cpp")

file(GLOB_RECURSE LIBRARY_HEADERS ${CMAKE_SOURCE_DIR}/include/**/*.h)

add_library(${LIBRARY_TARGET} ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
target_link_libraries(
  ${LIBRARY_TARGET}
  PRIVATE lua::lua
  PUBLIC Boost::headers Boost::filesystem Boost::process Python::Python
         $<$<BOOL:${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY}>:fmt::fmt>)
target_include_directories(
  ${LIBRARY_TARGET}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)
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
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION "include/")
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

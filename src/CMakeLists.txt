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
  PUBLIC Boost::filesystem
  PUBLIC Python::Python
  PRIVATE ${LUA_LIBRARIES})
if(${PPPLUGIN_ENABLE_CPP17_COMPATIBILITY})
  # TODO: generate C++17 compatible header files instead
  # target_compile_options(${LIBRARY_TARGET} PUBLIC
  # "-DPPPLUGIN_CPP17_COMPATIBILITY") # TODO: fix
  target_link_libraries(${LIBRARY_TARGET} PUBLIC fmt::fmt)
endif()
target_include_directories(
  ${LIBRARY_TARGET}
  PRIVATE ${LUA_INCLUDE_DIR}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

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

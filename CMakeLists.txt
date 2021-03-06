project(DockerClientpp)
cmake_minimum_required(VERSION 2.6)

option(ENABLE_TEST "enable testing" TRUE)
option(CI_TEST "indicates CI environment" OFF)
option(BUILD_SHARED_LIBS "build as a shared library" OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/DockerClientpp")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/libarchive/libarchive")

file(GLOB DOCKER_CLIENT_PP_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE DOCKER_CLIENT_PP_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

find_package(Threads REQUIRED)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/libarchive")

set(DOCKER_CLIENT_PP_LIB DockerClientpp)

add_library(${DOCKER_CLIENT_PP_LIB} ${DOCKER_CLIENT_PP_SRC_FILES})
set_target_properties(${DOCKER_CLIENT_PP_LIB}
  PROPERTIES
  CXX_STANDARD 14)
if (BUILD_SHARED_LIBS)
  target_link_libraries(${DOCKER_CLIENT_PP_LIB} archive)
else ()
  target_link_libraries(${DOCKER_CLIENT_PP_LIB} archive_static)
endif ()

install(TARGETS ${DOCKER_CLIENT_PP_LIB}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES ${DOCKER_CLIENT_PP_HEADER_FILES}
  DESTINATION "include/DockerClientpp")


if (ENABLE_TEST)
  enable_testing()

  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/googletest/googletest/include")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/googletest/googlemock/include")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/googletest")

  file(GLOB DOCKER_CLIENT_PP_TEST_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")

  set(CMAKE_BUILD_TYPE Debug)

  if (CI_TEST)
    add_definitions(-DCI_TEST)
  endif ()

  add_executable(${PROJECT_NAME}Test
    ${DOCKER_CLIENT_PP_TEST_SRC_FILES}
    ${DOCKER_CLIENT_PP_SRC_FILES})

  set_target_properties(${PROJECT_NAME}Test
    PROPERTIES
    CXX_STANDARD 14)

  target_link_libraries(${PROJECT_NAME}Test
    gmock
    ${CMAKE_THREAD_LIBS_INIT}
    archive)

  add_test(DockerClientppTest ${PROJECT_NAME}Test)
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} "Doxyfile"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generates API docs with doxygen"
    VERBATIM)
else ()
  message("Doxygen executable not found")
endif ()

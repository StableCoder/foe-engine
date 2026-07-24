# Copyright (C) 2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

set(STANDALONE_HEADER_COMPILE_IN
    ${CMAKE_CURRENT_LIST_DIR}/standalone_header_compile.in)

function(standalone_header_compile_test TARGET PATH_PREFIX)
  get_target_property(LIBRARY_HEADERS ${TARGET} HEADER_SET)
  get_target_property(LIBRARY_HEADER_DIRS ${TARGET} HEADER_DIRS)

  # go through each direct header
  unset(STANDALONE_HEADER_TEST_SOURCES)
  foreach(LIBRARY_HEADER ${LIBRARY_HEADERS})

    # figure out the relative header path from the include directories, ensuring
    # it has the expected prefix
    unset(RELATIVE_HEADER_PATH)
    foreach(INCLUDE_DIR ${LIBRARY_HEADER_DIRS})
      cmake_path(RELATIVE_PATH LIBRARY_HEADER BASE_DIRECTORY ${INCLUDE_DIR}
                 OUTPUT_VARIABLE RELATIVE_HEADER_PATH)
      cmake_path(IS_PREFIX PATH_PREFIX ${RELATIVE_HEADER_PATH} HAS_FOE_PREFIX)
      if(${HAS_FOE_PREFIX})
        break()
      endif()
      unset(RELATIVE_HEADER_PATH)
    endforeach()

    if(NOT DEFINED RELATIVE_HEADER_PATH)
      message(
        SEND_ERROR "Could not determine relative header path: ${LIBRARY_HEADER}"
      )
    endif()

    cmake_path(GET LIBRARY_HEADER STEM HEADER_STEM)
    cmake_path(GET LIBRARY_HEADER EXTENSION HEADER_EXTENSION)

    if(${HEADER_EXTENSION} STREQUAL ".h")
      # test in both C and CXX
      configure_file(
        ${STANDALONE_HEADER_COMPILE_IN}
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.c
      )
      list(
        APPEND
        STANDALONE_HEADER_TEST_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.c
      )
      configure_file(
        ${STANDALONE_HEADER_COMPILE_IN}
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.cpp
      )
      list(
        APPEND
        STANDALONE_HEADER_TEST_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.cpp
      )
    elseif(${HEADER_EXTENSION} STREQUAL ".hpp")
      # just test CXX
      configure_file(
        ${STANDALONE_HEADER_COMPILE_IN}
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.cpp
      )
      list(
        APPEND
        STANDALONE_HEADER_TEST_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/standalone_header_compile/${RELATIVE_HEADER_PATH}.cpp
      )
    else()
      message(
        SEND_ERROR "Unhandled header extension for file: ${LIBRARY_HEADER}")
    endif()
  endforeach()

  # add standalone library
  add_library(test_${TARGET}_standalone_header_compile
              ${STANDALONE_HEADER_TEST_SOURCES})

  set_target_properties(test_${TARGET}_standalone_header_compile
                        PROPERTIES FOLDER "StandaloneHeaderCompileTests")

  target_link_libraries(test_${TARGET}_standalone_header_compile
                        PRIVATE ${TARGET})
endfunction()

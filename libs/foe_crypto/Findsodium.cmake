#[=======================================================================[.rst:
Findsodium
----------

A library to import and export various 3d-model-formats including
scene-post-processing to generate missing render data.

http://sodium.org/

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` target ``sodium::sodium``, if
yaml-cpp has been found.


Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables::

  sodium_FOUND          - "True" if yaml-cpp was found
  sodium_INCLUDE_DIRS   - include directories for yaml-cpp
  sodium_LIBRARIES      - link against this library to use yaml-cpp

The module will also define three cache variables::

  sodium_INCLUDE_DIR        - the yaml-cpp include directory
  sodium_LIBRARY            - the path to the yaml-cpp library

#]=======================================================================]

find_path(sodium_INCLUDE_DIR NAMES sodium/core.h)
find_library(sodium_LIBRARY NAMES sodium libsodium)

set(sodium_INCLUDE_DIRS ${sodium_INCLUDE_DIR})
set(sodium_LIBRARIES ${sodium_LIBRARY})

find_package_handle_standard_args(sodium DEFAULT_MSG sodium_INCLUDE_DIR
                                  sodium_LIBRARY)

mark_as_advanced(sodium_INCLUDE_DIR sodium_LIBRARY)

if(sodium_FOUND AND NOT TARGET sodium::sodium)
  add_library(sodium::sodium UNKNOWN IMPORTED)
  set_target_properties(
    sodium::sodium
    PROPERTIES IMPORTED_LOCATION "${sodium_LIBRARIES}"
               INTERFACE_INCLUDE_DIRECTORIES "${sodium_INCLUDE_DIRS}")
endif()

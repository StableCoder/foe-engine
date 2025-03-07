# Copyright (C) 2023-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_crypto)

# Dependencies
if(WIN32)
  # On Windows, use the Findsodium module in this dir
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
  find_package(sodium REQUIRED)
else()
  # Otherwise use pkg-config
  find_package(PkgConfig)
  pkg_check_modules(libsodium REQUIRED libsodium)
endif()

# Declaration
add_library(foe_crypto SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_crypto EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/crypto/export.h)

target_include_directories(
  foe_crypto PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                    ${libsodium_INCLUDE_DIRS})

target_link_libraries(foe_crypto PUBLIC foe_core ${libsodium_LINK_LIBRARIES})
if(WIN32)
  target_link_libraries(foe_crypto PUBLIC foe_core sodium::sodium)
endif()

target_code_coverage(foe_crypto)

if(MSVC)
  target_compile_options(foe_crypto PRIVATE /EHsc)
else()
  target_compile_options(foe_crypto PRIVATE -fno-exceptions)
endif()

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

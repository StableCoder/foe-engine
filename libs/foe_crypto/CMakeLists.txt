# Copyright (C) 2023-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
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
add_library(foe::crypto ALIAS foe_crypto)

# Definition
generate_export_header(foe_crypto EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/crypto/export.h)

target_include_directories(foe_crypto PRIVATE ${libsodium_INCLUDE_DIRS})

target_link_libraries(
  foe_crypto
  PUBLIC foe_core
  PRIVATE ${libsodium_LINK_LIBRARIES})
if(WIN32)
  target_link_libraries(
    foe_crypto
    PUBLIC foe_core
    PRIVATE sodium::sodium)
endif()

target_code_coverage(foe_crypto)

if(MSVC)
  target_compile_options(foe_crypto PRIVATE /EHsc)
else()
  target_compile_options(foe_crypto PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_crypto
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/crypto/export.h
         include/foe/crypto/aes_256_gcm.h
         include/foe/crypto/ed25519.h
         include/foe/crypto/key.h
         include/foe/crypto/memory.h
         include/foe/crypto/random.h
         include/foe/crypto/result.h
         include/foe/crypto/sha256.h
         include/foe/crypto/sha512.h
         include/foe/crypto/x25519.h
         include/foe/crypto/xchacha20_poly1305.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_crypto PROPERTY EXPORT_NAME foe::crypto)

install(
  TARGETS foe_crypto
  EXPORT foe_crypto
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_crypto DESTINATION share/cmake/foe)

install(FILES foe_crypto-config.cmake DESTINATION share/cmake/foe)

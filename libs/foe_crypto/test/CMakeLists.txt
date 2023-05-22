# Copyright (C) 2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_crypto)
add_test(NAME FoE-Crypto-Test COMMAND test_foe_crypto)

# Definition
target_sources(
  test_foe_crypto
  PRIVATE aes_gcm.cpp
          c_header_compatibility.c
          ed25519.cpp
          key.cpp
          result.cpp
          sha256.cpp
          sha512.cpp
          x25519.cpp
          xchacha20_poly1305.cpp)

target_link_libraries(test_foe_crypto PRIVATE Catch2::Catch2WithMain foe_crypto)

target_code_coverage(
  test_foe_crypto
  AUTO
  ALL
  OBJECTS
  foe_crypto
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  ${BULLET_INCLUDE_DIRS})

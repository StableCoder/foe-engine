# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_crypto EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/crypto/export.h)

target_sources(
  foe_crypto
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/crypto/export.h
         foe/crypto/aes_256_gcm.h
         foe/crypto/ed25519.h
         foe/crypto/key.h
         foe/crypto/memory.h
         foe/crypto/random.h
         foe/crypto/result.h
         foe/crypto/sha256.h
         foe/crypto/sha512.h
         foe/crypto/x25519.h
         foe/crypto/xchacha20_poly1305.h)

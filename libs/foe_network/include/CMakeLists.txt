# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_network EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/network/export.h)

target_sources(
  foe_network
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/network/export.h
         foe/network/address.h
         foe/network/result.h
         foe/network/socket.h)

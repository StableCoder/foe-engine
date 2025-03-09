# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/yaml/export.h)

target_sources(
  foe_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/yaml/export.h
         foe/yaml/exception.hpp
         foe/yaml/glm_colour_parsing.hpp
         foe/yaml/glm.hpp
         foe/yaml/pod.hpp)

# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_imex_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/yaml/export.h)

target_sources(
  foe_imex_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/yaml/export.h
         foe/imex/yaml/exporter.hpp
         foe/imex/yaml/exporter_registration.h
         foe/imex/yaml/importer.hpp
         foe/imex/yaml/importer_registration.h
         foe/imex/yaml/result.h
         foe/imex/yaml/type_defs.h)

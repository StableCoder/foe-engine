# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_imex_yaml
  PRIVATE error_code.c
          exporter_registration.cpp
          exporter.cpp
          import_functionality.cpp
          importer_registration.cpp
          importer.cpp
          log.cpp)

# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_imex_binary
  PRIVATE exporter_registration.c
          exporter.cpp
          importer_functions.cpp
          importer_registration.c
          importer.cpp
          log.cpp
          result.c)

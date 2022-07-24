# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(foe_imex PRIVATE c_header_compatibility.c error_code.c
                                exporters.cpp importer.c importer.cpp log.cpp)

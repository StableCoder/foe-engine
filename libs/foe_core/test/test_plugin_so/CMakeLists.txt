# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Declaration
add_library(test_foe_core_plugin_so SHARED test.c)

set_target_properties(test_foe_core_plugin_so PROPERTIES FOLDER "Tests")

generate_export_header(test_foe_core_plugin_so EXPORT_MACRO_NAME TEST_EXPORT
                       EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/export.h)

target_include_directories(test_foe_core_plugin_so
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

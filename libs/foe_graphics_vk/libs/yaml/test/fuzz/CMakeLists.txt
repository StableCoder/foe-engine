# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

add_executable(fuzz_foe_graphics_vk_yaml fuzzer.cpp)
target_link_libraries(fuzz_foe_graphics_vk_yaml PRIVATE foe_graphics_vk_yaml)

add_executable(sort_fuzzed_foe_graphics_vk_yaml sorter.cpp)
target_link_libraries(sort_fuzzed_foe_graphics_vk_yaml
                      PRIVATE foe_graphics_vk_yaml)

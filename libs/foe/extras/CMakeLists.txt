# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Declaration
add_executable(foe_data_pool_benchmark data_pool_benchmark.cpp)
target_link_libraries(foe_data_pool_benchmark PRIVATE foe)

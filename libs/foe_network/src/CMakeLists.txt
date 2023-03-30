# Copyright (C) 2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(foe_network PRIVATE address.c result.c socket.c)

if(WIN32)
  target_sources(foe_network PRIVATE network_initialization_win32.cpp)
else()
  target_sources(foe_network PRIVATE network_initialization_unix.c)
endif()

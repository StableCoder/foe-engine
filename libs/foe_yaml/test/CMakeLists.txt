# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Yaml-Test COMMAND test_foe_yaml)
add_executable(test_foe_yaml)

# Definition
target_sources(test_foe_yaml PRIVATE exception.cpp pod_bool_parsing.cpp
                                     pod_int16_parsing.cpp string_parsing.cpp)

target_link_libraries(test_foe_yaml PRIVATE catch foe_yaml)

if(UNIX
   OR CMAKE_C_COMPILER_ID MATCHES "[Cc]lang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang")
  target_compile_options(test_foe_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(test_foe_yaml PRIVATE /EHa)
endif()

target_code_coverage(
  test_foe_yaml
  AUTO
  ALL
  OBJECTS
  foe_yaml
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)

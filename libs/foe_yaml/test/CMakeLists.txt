# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_yaml exception.cpp pod_bool_parsing.cpp
                             pod_int16_parsing.cpp)
add_test(NAME FoE-Yaml-Test COMMAND test_foe_yaml)

target_link_libraries(test_foe_yaml PRIVATE catch foe_yaml)
target_code_coverage(
  test_foe_yaml
  AUTO
  ALL
  OBJECTS
  foe_wsi
  EXCLUDE
  .*/test/.*)

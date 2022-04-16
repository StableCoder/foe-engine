# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_imex_yaml)
add_test(NAME FoE-ImEx-Yaml-Test COMMAND test_foe_imex_yaml)

# Definition
target_sources(
  test_foe_imex_yaml
  PRIVATE c_header_compatability.c
          create_importer.cpp
          error_code.cpp
          imex_registration.cpp
          importer.cpp
          test_importer.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_compile_definitions(
  test_foe_imex_yaml PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(test_foe_imex_yaml PRIVATE catch foe_imex_yaml)

target_code_coverage(
  test_foe_imex_yaml
  AUTO
  ALL
  OBJECTS
  foe_imex_yaml
  EXCLUDE
  .*/test/.*)

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_imex)
add_test(NAME FoE-ImEx-Test COMMAND test_foe_imex)

# Definition
target_sources(
  test_foe_imex
  PRIVATE error_code.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_imex PRIVATE catch foe_imex)

target_code_coverage(
  test_foe_imex
  AUTO
  ALL
  OBJECTS
  foe_imex
  EXCLUDE
  .*/test/.*)

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_physics)
add_test(NAME FoE-Physics-Test COMMAND test_foe_physics)

# Definition
target_sources(
  test_foe_physics
  PRIVATE error_code.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_physics PRIVATE catch foe_physics)

target_code_coverage(
  test_foe_physics
  AUTO
  ALL
  OBJECTS
  foe_physics
  EXCLUDE
  .*/test/.*)

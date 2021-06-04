# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_simulation group_data.cpp)
add_test(NAME FoE-Simulation-Test COMMAND test_foe_simulation)

target_include_directories(test_foe_simulation PRIVATE ../src)
target_link_libraries(test_foe_simulation PRIVATE catch foe_engine_lib)
target_code_coverage(
  test_foe_simulation
  AUTO
  ALL
  OBJECTS
  foe_engine_lib
  EXCLUDE
  .*/test/.*)

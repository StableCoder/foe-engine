# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_simulation group_data.cpp simulation.cpp)
add_test(NAME FoE-Simulation-Test COMMAND test_foe_simulation)

target_compile_definitions(
  test_foe_simulation
  PRIVATE FOE_SIMULATION_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
target_link_libraries(test_foe_simulation PRIVATE catch foe_simulation)
target_code_coverage(
  test_foe_simulation
  AUTO
  ALL
  OBJECTS
  foe_simulation
  EXCLUDE
  .*/test/.*)

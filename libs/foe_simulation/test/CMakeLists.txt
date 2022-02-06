# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_simulation)
add_test(NAME FoE-Simulation-Test COMMAND test_foe_simulation)

# Definition
target_sources(
  test_foe_simulation
  PRIVATE error_code.cpp
          group_data.cpp
          simulation.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

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

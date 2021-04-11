# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_engine group_data.cpp)
add_test(NAME FoE-Engine-Library-Test COMMAND test_foe_engine)

target_include_directories(test_foe_engine PRIVATE ../src)
target_link_libraries(test_foe_engine PRIVATE catch foe_engine_lib)
target_code_coverage(
  test_foe_engine
  AUTO
  ALL
  OBJECTS
  foe_engine_lib
  EXCLUDE
  .*/test/.*)

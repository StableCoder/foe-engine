# Setup
prepare_catch(COMPILED_CATCH)

# Executable
add_executable(test_foe log_category.cpp)
add_test(NAME FoE-Test COMMAND test_foe)

target_link_libraries(test_foe PRIVATE catch foe)
target_code_coverage(
  test_foe
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Definition
add_subdirectory(chrono)

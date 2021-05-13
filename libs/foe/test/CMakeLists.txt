# Setup
prepare_catch(COMPILED_CATCH)

# Main Declaration
add_executable(
  test_foe
  command_string_runner.cpp
  data_pool_custom.cpp
  data_pool_int.cpp
  data_pool_multi.cpp
  filesystem.cpp
  plugin.cpp
  search_paths.cpp)
add_test(NAME FoE-Test COMMAND test_foe)

target_compile_definitions(
  test_foe PRIVATE TEST_PLUGIN_DIR="$<TARGET_FILE:test_foe_plugin_so>"
                   TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(test_foe PRIVATE catch foe)
target_code_coverage(
  test_foe
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Thread
add_executable(test_foe_thread thread_pool.cpp)
add_test(NAME FoE-Test-Thread COMMAND test_foe_thread)

target_link_libraries(test_foe_thread PRIVATE catch foe)
target_code_coverage(
  test_foe_thread
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Definition
add_subdirectory(chrono)
add_subdirectory(log)
add_subdirectory(storage)
add_subdirectory(test_plugin_so)

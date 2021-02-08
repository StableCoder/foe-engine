# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe command_string_runner.cpp filesystem.cpp plugin.cpp
                        thread_pool.cpp)
add_test(NAME FoE-Test COMMAND test_foe)

target_compile_definitions(
  test_foe PRIVATE TEST_PLUGIN_DIR="$<TARGET_FILE:test_foe_plugin_so>")

target_link_libraries(test_foe PRIVATE catch foe)
if(UNIX)
  target_link_libraries(test_foe PRIVATE pthread)
endif()
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
add_subdirectory(log)
add_subdirectory(test_plugin_so)

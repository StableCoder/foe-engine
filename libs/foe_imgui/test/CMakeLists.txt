# Test Setup
enable_testing()
prepare_catch(COMPILED_CATCH)

add_executable(test_foe_imgui state.cpp)
target_link_libraries(test_foe_imgui PRIVATE foe_imgui catch)
target_code_coverage(
  test_foe_imgui
  AUTO
  ALL
  EXCLUDE
  .*/external/.*
  .*/test/.*
  OBJECTS
  foe_imgui)

add_test(NAME FoE-ImGui-Tests COMMAND test_foe_imgui)

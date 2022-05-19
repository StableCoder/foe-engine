# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-ImGui-VK-Test COMMAND test_foe_imgui_vk)
add_executable(test_foe_imgui_vk)

# Definition
target_sources(
  test_foe_imgui_vk
  PRIVATE c_header_compatability.c
          error_code.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_imgui_vk PRIVATE catch foe_imgui_vk)

target_code_coverage(
  test_foe_imgui_vk
  AUTO
  ALL
  OBJECTS
  foe_imgui_vk
  EXCLUDE
  .*/test/.*)

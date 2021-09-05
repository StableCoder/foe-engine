# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Graphics-Vk-Test COMMAND test_foe_graphics_vk)
add_executable(test_foe_graphics_vk)

# Definition
target_sources(test_foe_graphics_vk PRIVATE sample_count.cpp)

target_link_libraries(test_foe_graphics_vk PRIVATE catch foe_graphics_vk)

target_code_coverage(
  test_foe_graphics_vk
  AUTO
  ALL
  OBJECTS
  foe_graphics_vk
  EXCLUDE
  .*/test/.*)

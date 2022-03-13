# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Graphics-VK-Test COMMAND test_foe_graphics_vk)
add_executable(test_foe_graphics_vk)

# Definition
target_sources(
  test_foe_graphics_vk
  PRIVATE c_header_compatability.c
          error_code.cpp
          sample_count.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_graphics_vk PRIVATE catch foe_graphics_vk)

target_code_coverage(
  test_foe_graphics_vk
  AUTO
  ALL
  OBJECTS
  foe_graphics_vk
  EXCLUDE
  .*/test/.*)

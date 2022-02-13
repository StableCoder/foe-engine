# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-OpenXR-VK-Test COMMAND test_foe_xr_openxr_vk)
add_executable(test_foe_xr_openxr_vk)

# Definition
target_sources(
  test_foe_xr_openxr_vk
  PRIVATE error_code.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_xr_openxr_vk PRIVATE catch foe_xr_openxr_vk)

target_code_coverage(
  test_foe_xr_openxr_vk
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr_vk
  EXCLUDE
  .*/test/.*)

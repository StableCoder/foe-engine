# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-OpenXR-Test COMMAND test_foe_openxr)
add_executable(test_foe_openxr)

# Definition
target_sources(
  test_foe_openxr
  PRIVATE error_code.cpp
          openxr_error_code.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_openxr PRIVATE catch foe_xr_openxr)

target_code_coverage(
  test_foe_openxr
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr
  EXCLUDE
  .*/test/.*)

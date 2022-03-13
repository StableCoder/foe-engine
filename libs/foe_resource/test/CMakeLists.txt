# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_resource)
add_test(NAME FoE-Resource-Test COMMAND test_foe_resource)

# Definition
target_sources(
  test_foe_resource
  PRIVATE c_header_compatability.c
          create_info.cpp
          error_code.cpp
          resource.cpp
          resource_importing_loading.cpp
          # The following set are sources that needs to be recompiled as the
          # symbols may not be exported or have been stripped from  thelinked
          # libraries
          ../src/error_code.cpp)

target_link_libraries(test_foe_resource PRIVATE catch foe_resource)

target_code_coverage(
  test_foe_resource
  AUTO
  ALL
  OBJECTS
  foe_resource
  EXCLUDE
  .*/test/.*)

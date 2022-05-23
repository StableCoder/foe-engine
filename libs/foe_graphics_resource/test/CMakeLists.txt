# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_graphics_resource)
add_test(NAME FoE-Graphics-Resource-Test COMMAND test_foe_physics)

# Definition
target_sources(test_foe_graphics_resource PRIVATE c_header_compatability.c
                                                  error_code.cpp)

target_link_libraries(test_foe_graphics_resource PRIVATE catch
                                                         foe_graphics_resource)

target_code_coverage(
  test_foe_graphics_resource
  AUTO
  ALL
  OBJECTS
  foe_graphics_resource
  EXCLUDE
  .*/test/.*)

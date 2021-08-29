# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-ECS-Test COMMAND test_foe_ecs)
add_executable(test_foe_ecs)

# Definition
target_sources(test_foe_ecs PRIVATE editor_name_map.cpp group_translator.cpp
                                    id.cpp index_generator.cpp)

target_link_libraries(test_foe_ecs PRIVATE catch foe_ecs)

target_code_coverage(
  test_foe_ecs
  AUTO
  ALL
  OBJECTS
  foe_ecs
  EXCLUDE
  .*/test/.*)

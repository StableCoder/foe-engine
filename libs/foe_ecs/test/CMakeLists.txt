# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_ecs editor_name_map.cpp group_translator.cpp id.cpp
                            index_generator.cpp)
add_test(NAME FoE-ECS-Test COMMAND test_foe_ecs)

target_link_libraries(test_foe_ecs PRIVATE catch foe_ecs)
if(UNIX)
  target_link_libraries(test_foe_ecs PRIVATE pthread)
endif()
target_code_coverage(
  test_foe_ecs
  AUTO
  ALL
  OBJECTS
  foe_ecs
  EXCLUDE
  .*/test/.*)

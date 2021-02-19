# Test Setup
enable_testing()
prepare_catch(COMPILED_CATCH)

# Executables
add_executable(test_foe_ecs_yaml index_generator.cpp)

target_link_libraries(test_foe_ecs_yaml PRIVATE foe_ecs_yaml catch)
target_code_coverage(
  test_foe_ecs_yaml
  AUTO
  ALL
  EXCLUDE
  /usr/*
  .*.test..*
  .*/test/.*
  .*/tests/.*
  OBJECTS
  foe_ecs)

add_test(NAME FoE-ECS-Yaml-Tests COMMAND test_foe_ecs_yaml)

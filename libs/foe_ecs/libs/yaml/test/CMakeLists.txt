# Test Setup
enable_testing()
prepare_catch(COMPILED_CATCH)

# Executables
add_executable(test_foe_ecs_yaml id.cpp index_generator.cpp)

target_link_libraries(test_foe_ecs_yaml PRIVATE foe_ecs_yaml catch)

if(UNIX)
  target_compile_options(test_foe_ecs_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(test_foe_ecs_yaml PRIVATE /EHa)
endif()

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
  foe_ecs_yaml)

add_test(NAME FoE-ECS-Yaml-Tests COMMAND test_foe_ecs_yaml)

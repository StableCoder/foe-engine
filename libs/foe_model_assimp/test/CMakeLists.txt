# Test Setup
enable_testing()
prepare_catch(COMPILED_CATCH)

add_executable(test_foe_model_assimp flags.cpp)
target_link_libraries(test_foe_model_assimp PRIVATE foe_model_assimp catch)
target_code_coverage(
  test_foe_model_assimp
  AUTO
  ALL
  EXCLUDE
  .*/external/.*
  .*/test/.*
  OBJECTS
  foe_model_assimp)

add_test(NAME FoE-Model-Assimp-Tests COMMAND test_foe_model_assimp)

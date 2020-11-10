# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(
  test_foe_graphics_yaml
  vk_struct_VkPipelineDepthStencilStateCreateInfo.cpp
  vk_struct_VkStencilOpState.cpp
  vk_type_parsing_optional.cpp
  vk_type_parsing_required.cpp
  ../src/vk_type_parsing.cpp
  ../src/vk_struct_parsing.cpp)
add_test(NAME FoE-Graphics-Yaml-Test COMMAND test_foe_graphics_yaml)

target_link_libraries(test_foe_graphics_yaml PRIVATE catch foe_graphics_yaml)
target_code_coverage(
  test_foe_graphics_yaml
  AUTO
  ALL
  OBJECTS
  foe_wsi
  EXCLUDE
  .*/test/.*)

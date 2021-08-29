# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(
  test_foe_graphics_vk_yaml
  vk_struct_VkPipelineDepthStencilStateCreateInfo.cpp
  vk_struct_VkStencilOpState.cpp vk_type_parsing_optional.cpp
  vk_type_parsing_required.cpp)
add_test(NAME FoE-Graphics-Yaml-Test COMMAND test_foe_graphics_vk_yaml)

target_link_libraries(test_foe_graphics_vk_yaml PRIVATE catch
                                                        foe_graphics_vk_yaml)

if(UNIX)
  target_compile_options(test_foe_graphics_vk_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(test_foe_graphics_vk_yaml PRIVATE /EHa)
endif()

target_code_coverage(
  test_foe_graphics_vk_yaml
  AUTO
  ALL
  OBJECTS
  foe_wsi
  EXCLUDE
  .*/test/.*)

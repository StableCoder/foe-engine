add_subdirectory(foe)
add_subdirectory(foe_ecs)
add_subdirectory(foe_graphics)
add_subdirectory(foe_graphics_vk)
add_subdirectory(foe_imex)
add_subdirectory(foe_imgui)
add_subdirectory(foe_model)
add_subdirectory(foe_model_importer_assimp)
add_subdirectory(foe_physics)
add_subdirectory(foe_position)
add_subdirectory(foe_resource)
add_subdirectory(foe_simulation)
add_subdirectory(foe_wsi)
add_subdirectory(foe_xr)
add_subdirectory(foe_yaml)

if(BUILD_XR)
  add_subdirectory(foe_xr_openxr)
endif()

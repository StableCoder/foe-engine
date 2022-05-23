target_sources(
  foe_graphics_resource
  PRIVATE c_header_compatability.c
          error_code.c
          image_create_info.cpp
          image_loader.cpp
          log.cpp
          material_create_info.cpp
          material_loader.cpp
          mesh_create_info.cpp
          mesh_loader.cpp
          registration.cpp
          shader_create_info.cpp
          shader_loader.cpp
          vertex_descriptor_create_info.cpp
          vertex_descriptor_loader.cpp
          worst_resource_state.cpp
          vk_result.c)

target_sources(
  foe_bringup_lib_eh
  PRIVATE armature_loader.cpp
          armature_pool.cpp
          armature_system.cpp
          camera_system.cpp
          position_descriptor_pool.cpp
          registration.cpp
          vk_animation.cpp)

add_subdirectory(yaml)

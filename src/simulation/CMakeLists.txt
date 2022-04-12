target_sources(
  foe_bringup_shared
  PRIVATE ../log.cpp
          ../error_code.cpp
          armature_loader.cpp
          armature_pool.cpp
          armature_system.cpp
          camera_system.cpp
          position_descriptor_pool.cpp
          registration.cpp
          vk_animation.cpp)

add_subdirectory(yaml)

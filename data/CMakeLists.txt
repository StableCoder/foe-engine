# A regular 'shaders' target that can be called manually to do *all* shaders
if(WIN32)
  add_custom_target(
    shaders
    COMMAND powershell ${CMAKE_CURRENT_SOURCE_DIR}/compile_shaders.ps1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  add_custom_target(
    shaders
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile_shaders.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

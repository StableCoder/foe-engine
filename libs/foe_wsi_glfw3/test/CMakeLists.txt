# Setup
prepare_catch(COMPILED_CATCH)

if(BUILD_WINDOW_TESTS)
  # Declaration
  add_executable(test_foe_wsi_glfw3 window.cpp)
  add_test(NAME FoE-WSI-GLFW3-Test COMMAND test_foe_wsi_glfw3)

  if(BUILD_VULKAN_RUNTIME_TESTS)
    target_sources(test_foe_wsi_glfw3 PRIVATE vulkan.cpp)
  endif()

  target_include_directories(test_foe_wsi_glfw3
                             PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../private)

  target_link_libraries(test_foe_wsi_glfw3 PRIVATE catch foe_wsi_glfw3)
  target_code_coverage(
    test_foe_wsi_glfw3
    AUTO
    ALL
    OBJECTS
    foe_wsi_glfw3
    EXCLUDE
    .*/test/.*)
endif()

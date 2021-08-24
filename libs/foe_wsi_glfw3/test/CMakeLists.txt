# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_wsi_glfw3 keyboard.cpp mouse.cpp)
add_test(NAME FoE-WSI-GLFW3-Test COMMAND test_foe_wsi_glfw3)

target_link_libraries(test_foe_wsi_glfw3 PRIVATE catch foe_wsi_glfw3)
target_code_coverage(
  test_foe_wsi_glfw3
  AUTO
  ALL
  OBJECTS
  foe_wsi
  EXCLUDE
  .*/test/.*)

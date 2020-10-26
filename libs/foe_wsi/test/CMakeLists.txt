# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_wsi keyboard.cpp mouse.cpp)
add_test(NAME FoE-WSI-Test COMMAND test_foe_wsi)

target_link_libraries(test_foe_wsi PRIVATE catch foe_wsi)
target_code_coverage(
  test_foe_wsi
  AUTO
  ALL
  OBJECTS
  foe_wsi
  EXCLUDE
  .*/test/.*)

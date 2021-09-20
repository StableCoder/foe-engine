add_executable(example_foe_wsi_glfw_simple simple.cpp)

target_include_directories(example_foe_wsi_glfw_simple
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../private)
target_link_libraries(example_foe_wsi_glfw_simple PRIVATE foe_wsi_glfw3)

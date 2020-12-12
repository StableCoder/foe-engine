# Declaration
add_library(test_foe_plugin_so SHARED test.c)
generate_export_header(test_foe_plugin_so EXPORT_MACRO_NAME TEST_EXPORT
                       EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/export.h)

target_include_directories(test_foe_plugin_so
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

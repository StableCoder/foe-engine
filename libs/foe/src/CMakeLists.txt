target_sources(foe PRIVATE command_string_runner.cpp developer_console.cpp
                           log.cpp plugin.cpp thread_pool.cpp)

add_subdirectory(chrono)
add_subdirectory(log)

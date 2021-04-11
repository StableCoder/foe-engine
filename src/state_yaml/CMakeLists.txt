target_sources(foe_engine_lib PRIVATE distributed_yaml.cpp entity.cpp)

target_link_libraries(foe_engine_lib PRIVATE foe_yaml)

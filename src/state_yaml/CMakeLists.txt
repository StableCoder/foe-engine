target_sources(foe_engine PRIVATE distributed_yaml.cpp entity.cpp export.cpp
                                  import.cpp)

target_link_libraries(foe_engine PRIVATE foe_yaml)

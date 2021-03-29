target_sources(foe_engine PRIVATE entity.cpp export.cpp import.cpp)

target_link_libraries(foe_engine PRIVATE foe_ecs_yaml)

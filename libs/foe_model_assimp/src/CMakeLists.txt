# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_model_assimp PRIVATE assimp_mesh_loader.cpp assimp_scene_loader.cpp
                           flags.cpp importer.cpp)

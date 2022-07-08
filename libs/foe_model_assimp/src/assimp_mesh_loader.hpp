// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ASSIMP_MESH_LOADER_HPP
#define ASSIMP_MESH_LOADER_HPP

#include <assimp/scene.h>
#include <foe/model/armature.hpp>
#include <foe/model/vertex_component.hpp>
#include <glm/glm.hpp>

#include <vector>

void importMeshVertexData(aiMesh const *pMesh,
                          uint32_t componentCount,
                          foeVertexComponent const *pComponents,
                          glm::mat4 const &transform,
                          float *pData);

auto importMeshVertexData(aiMesh const *pMesh,
                          uint32_t componentCount,
                          foeVertexComponent const *pComponents,
                          glm::mat4 const &transform) -> std::vector<float>;

void importMeshIndexData16(aiMesh const *pMesh, uint16_t offset, uint16_t *pData);

auto importMeshIndexData16(aiMesh const *pMesh, uint16_t offset) -> std::vector<uint16_t>;

void importMeshIndexData32(aiMesh const *pMesh, uint32_t offset, uint32_t *pData);

auto importMeshIndexData32(aiMesh const *pMesh, uint32_t offset) -> std::vector<uint32_t>;

auto getMeshVerticesByWeight(aiMesh const *pMesh) -> std::vector<int>;

auto getMeshMaxPerVertexWeights(aiMesh const *pMesh) -> int;

void importMeshVertexBoneWeights(aiMesh const *pMesh, int maxPerVertexWeights, void *pData);

auto importMeshBones(aiMesh const *pMesh) -> std::vector<foeMeshBone>;

#endif // ASSIMP_MESH_LOADER_HPP
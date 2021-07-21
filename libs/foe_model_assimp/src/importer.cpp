/*
    Copyright (C) 2020-2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/model/assimp/importer.hpp>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "assimp_mesh_loader.hpp"
#include "assimp_scene_loader.hpp"

foeModelAssimpImporter::foeModelAssimpImporter(std::string_view filePath) : mpScene{nullptr} {
    mpScene = aiImportFile(std::string{filePath}.c_str(), 0);
}

foeModelAssimpImporter::~foeModelAssimpImporter() {
    if (mpScene != nullptr) {
        aiReleaseImport(mpScene);
    }
}

bool foeModelAssimpImporter::loaded() const noexcept { return mpScene != nullptr; }

uint32_t foeModelAssimpImporter::getNumMeshes() const noexcept { return mpScene->mNumMeshes; }

auto foeModelAssimpImporter::getMeshName(unsigned int mesh) const noexcept -> std::string_view {
    return mpScene->mMeshes[mesh]->mName.C_Str();
}

uint32_t foeModelAssimpImporter::getNumMeshVertices(unsigned int mesh) const noexcept {
    return mpScene->mMeshes[mesh]->mNumVertices;
}

auto foeModelAssimpImporter::importArmature() -> std::vector<foeArmatureNode> {
    return importSceneArmature(mpScene);
}

uint32_t foeModelAssimpImporter::getNumAnimations() const noexcept {
    return mpScene->mNumAnimations;
}

auto foeModelAssimpImporter::getAnimationName(unsigned int animation) const noexcept
    -> std::string_view {
    return mpScene->mAnimations[animation]->mName.C_Str();
}

auto foeModelAssimpImporter::importAnimation(unsigned int animation) -> foeAnimation {
    return ::importAnimation(mpScene->mAnimations[animation]);
}

void foeModelAssimpImporter::importMeshVertexData(unsigned int mesh,
                                                  uint32_t componentCount,
                                                  foeVertexComponent const *pComponents,
                                                  glm::mat4 const &transform,
                                                  float *pData) {
    ::importMeshVertexData(mpScene->mMeshes[mesh], componentCount, pComponents, transform, pData);
}

auto foeModelAssimpImporter::importMeshVertexData(unsigned int mesh,
                                                  uint32_t componentCount,
                                                  foeVertexComponent const *pComponents,
                                                  glm::mat4 const &transform)
    -> std::vector<float> {
    return ::importMeshVertexData(mpScene->mMeshes[mesh], componentCount, pComponents, transform);
}

uint32_t foeModelAssimpImporter::getNumFaces(unsigned int mesh) const noexcept {
    return mpScene->mMeshes[mesh]->mNumFaces;
}

void foeModelAssimpImporter::importMeshIndexData16(unsigned int mesh,
                                                   uint16_t offset,
                                                   uint16_t *pData) {
    ::importMeshIndexData16(mpScene->mMeshes[mesh], offset, pData);
}

auto foeModelAssimpImporter::importMeshIndexData16(unsigned int mesh, uint16_t offset)
    -> std::vector<uint16_t> {
    return ::importMeshIndexData16(mpScene->mMeshes[mesh], offset);
}

void foeModelAssimpImporter::importMeshIndexData32(unsigned int mesh,
                                                   uint32_t offset,
                                                   uint32_t *pData) {
    ::importMeshIndexData32(mpScene->mMeshes[mesh], offset, pData);
}

auto foeModelAssimpImporter::importMeshIndexData32(unsigned int mesh, uint32_t offset)
    -> std::vector<uint32_t> {
    return ::importMeshIndexData32(mpScene->mMeshes[mesh], offset);
}

auto foeModelAssimpImporter::getMeshVerticesByWeight(unsigned int mesh) -> std::vector<int> {
    return ::getMeshVerticesByWeight(mpScene->mMeshes[mesh]);
}

auto foeModelAssimpImporter::getMeshMaxPerVertexWeights(unsigned int mesh) -> int {
    return ::getMeshMaxPerVertexWeights(mpScene->mMeshes[mesh]);
}

void foeModelAssimpImporter::importMeshVertexBoneWeights(unsigned int mesh,
                                                         int maxPerVertexWeights,
                                                         void *pData) {
    ::importMeshVertexBoneWeights(mpScene->mMeshes[mesh], maxPerVertexWeights, pData);
}

auto foeModelAssimpImporter::importMeshBones(unsigned int mesh) -> std::vector<foeMeshBone> {
    return ::importMeshBones(mpScene->mMeshes[mesh]);
}

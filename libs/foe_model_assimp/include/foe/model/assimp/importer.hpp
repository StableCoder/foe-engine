// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_ASSIMP_IMPORTER_HPP
#define FOE_MODEL_ASSIMP_IMPORTER_HPP

#include <foe/model/assimp/export.h>
#include <foe/model/importer.hpp>

struct aiScene;

class FOE_MODEL_ASSIMP_EXPORT foeModelAssimpImporter final : public foeModelImporter {
  public:
    foeModelAssimpImporter(void *pMemory,
                           uint32_t memorySize,
                           std::string_view filePath,
                           unsigned int postProcessFlags);
    ~foeModelAssimpImporter();

    bool loaded() const noexcept;

    uint32_t getNumMeshes() const noexcept;

    auto getMeshName(unsigned int mesh) const noexcept -> std::string_view;
    uint32_t getNumMeshVertices(unsigned int mesh) const noexcept;

    auto importArmature() -> std::vector<foeArmatureNode>;

    uint32_t getNumAnimations() const noexcept;
    auto getAnimationName(unsigned int animation) const noexcept -> std::string_view;
    auto importAnimation(unsigned int animation) -> foeAnimation;

    void importMeshVertexData(unsigned int mesh,
                              uint32_t componentCount,
                              foeVertexComponent const *pComponents,
                              glm::mat4 const &transform,
                              float *pData);

    auto importMeshVertexData(unsigned int mesh,
                              uint32_t componentCount,
                              foeVertexComponent const *pComponents,
                              glm::mat4 const &transform) -> std::vector<float>;

    uint32_t getNumFaces(unsigned int mesh) const noexcept;

    void importMeshIndexData16(unsigned int mesh, uint16_t offset, uint16_t *pData);

    auto importMeshIndexData16(unsigned int mesh, uint16_t offset) -> std::vector<uint16_t>;

    void importMeshIndexData32(unsigned int mesh, uint32_t offset, uint32_t *pData);

    auto importMeshIndexData32(unsigned int mesh, uint32_t offset) -> std::vector<uint32_t>;

    auto getMeshVerticesByWeight(unsigned int mesh) -> std::vector<int>;

    auto getMeshMaxPerVertexWeights(unsigned int mesh) -> int;

    void importMeshVertexBoneWeights(unsigned int mesh, int maxPerVertexWeights, void *pData);

    auto importMeshBones(unsigned int mesh) -> std::vector<foeMeshBone>;

  private:
    FOE_MODEL_ASSIMP_NO_EXPORT aiScene const *mpScene;
};

#endif // FOE_MODEL_ASSIMP_IMPORTER_HPP
/*
    Copyright (C) 2020-2022 George Cave.

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

#ifndef FOE_MODEL_ASSIMP_IMPORTER_HPP
#define FOE_MODEL_ASSIMP_IMPORTER_HPP

#include <foe/model/assimp/export.h>
#include <foe/model/importer.hpp>

struct aiScene;

class FOE_MODEL_ASSIMP_EXPORT foeModelAssimpImporter final : public foeModelImporter {
  public:
    foeModelAssimpImporter(std::string_view filePath, unsigned int postProcessFlags);
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
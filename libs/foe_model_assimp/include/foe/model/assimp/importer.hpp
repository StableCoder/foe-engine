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

#ifndef FOE_MODEL_ASSIMP_IMPORTER_HPP
#define FOE_MODEL_ASSIMP_IMPORTER_HPP

#include <foe/model/assimp/export.h>
#include <foe/model/importer.hpp>

struct aiScene;

class foeModelAssimpImporter final : public foeModelImporter {
  public:
    FOE_MODEL_ASSIMP_EXPORT foeModelAssimpImporter(std::string_view filePath);
    FOE_MODEL_ASSIMP_EXPORT ~foeModelAssimpImporter();

    FOE_MODEL_ASSIMP_EXPORT bool loaded() const noexcept;

    FOE_MODEL_ASSIMP_EXPORT uint32_t getNumMeshes() const noexcept;

    FOE_MODEL_ASSIMP_EXPORT auto getMeshName(unsigned int mesh) const noexcept -> std::string_view;
    FOE_MODEL_ASSIMP_EXPORT uint32_t getNumMeshVertices(unsigned int mesh) const noexcept;

    FOE_MODEL_ASSIMP_EXPORT auto importArmature() -> std::vector<foeArmatureNode>;

    FOE_MODEL_ASSIMP_EXPORT uint32_t getNumAnimations() const noexcept;
    FOE_MODEL_ASSIMP_EXPORT auto getAnimationName(unsigned int animation) const noexcept
        -> std::string_view;
    FOE_MODEL_ASSIMP_EXPORT auto importAnimation(unsigned int animation) -> foeAnimation;

    FOE_MODEL_ASSIMP_EXPORT void importMeshVertexData(unsigned int mesh,
                                                      uint32_t componentCount,
                                                      foeVertexComponent const *pComponents,
                                                      glm::mat4 const &transform,
                                                      float *pData);

    FOE_MODEL_ASSIMP_EXPORT auto importMeshVertexData(unsigned int mesh,
                                                      uint32_t componentCount,
                                                      foeVertexComponent const *pComponents,
                                                      glm::mat4 const &transform)
        -> std::vector<float>;

    FOE_MODEL_ASSIMP_EXPORT uint32_t getNumFaces(unsigned int mesh) const noexcept;

    FOE_MODEL_ASSIMP_EXPORT void importMeshIndexData16(unsigned int mesh,
                                                       uint16_t offset,
                                                       uint16_t *pData);

    FOE_MODEL_ASSIMP_EXPORT auto importMeshIndexData16(unsigned int mesh, uint16_t offset)
        -> std::vector<uint16_t>;

    FOE_MODEL_ASSIMP_EXPORT void importMeshIndexData32(unsigned int mesh,
                                                       uint32_t offset,
                                                       uint32_t *pData);

    FOE_MODEL_ASSIMP_EXPORT auto importMeshIndexData32(unsigned int mesh, uint32_t offset)
        -> std::vector<uint32_t>;

    FOE_MODEL_ASSIMP_EXPORT auto getMeshVerticesByWeight(unsigned int mesh) -> std::vector<int>;

    FOE_MODEL_ASSIMP_EXPORT auto getMeshMaxPerVertexWeights(unsigned int mesh) -> int;

    FOE_MODEL_ASSIMP_EXPORT void importMeshVertexBoneWeights(unsigned int mesh,
                                                             int maxPerVertexWeights,
                                                             void *pData);

    FOE_MODEL_ASSIMP_EXPORT auto importMeshBones(unsigned int mesh) -> std::vector<foeMeshBone>;

  private:
    aiScene const *mpScene;
};

#endif // FOE_MODEL_ASSIMP_IMPORTER_HPP
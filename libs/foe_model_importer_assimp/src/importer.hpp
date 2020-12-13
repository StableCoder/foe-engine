/*
    Copyright (C) 2020 George Cave.

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

#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <foe/model/importer.hpp>

struct aiScene;

class foeModelFileAssimpImporter final : public foeModelImporter {
  public:
    foeModelFileAssimpImporter(std::string_view filePath);
    ~foeModelFileAssimpImporter();

    bool loaded() const noexcept;

    uint32_t getNumMeshVertices(unsigned int mesh) const noexcept;

    auto importArmature() -> std::vector<foeArmatureNode>;
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

    void importMeshIndexData16(unsigned int mesh, uint16_t offset, uint16_t *pData);

    auto importMeshIndexData16(unsigned int mesh, uint16_t offset) -> std::vector<uint16_t>;

    void importMeshIndexData32(unsigned int mesh, uint32_t offset, uint32_t *pData);

    auto importMeshIndexData32(unsigned int mesh, uint32_t offset) -> std::vector<uint32_t>;

    auto getMeshVerticesByWeight(unsigned int mesh) -> std::vector<int>;

    auto getMeshMaxPerVertexWeights(unsigned int mesh) -> int;

    void importMeshVertexBoneWeights(unsigned int mesh, int maxPerVertexWeights, void *pData);

    auto importMeshBones(unsigned int mesh) -> std::vector<foeMeshBone>;

  private:
    aiScene const *mpScene;
};

#endif // IMPORTER_HPP
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

#ifndef FOE_MODEL_IMPORTER_HPP
#define FOE_MODEL_IMPORTER_HPP

#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>
#include <foe/model/vertex_component.hpp>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

/**
 * @brief A pure-virtual base class for providing 3D model/mesh data
 *
 * Can be derived for importing data from files, from remote locations or generating data
 * on-the-fly.
 */
class foeModelImporter {
  public:
    virtual ~foeModelImporter() = default;

    virtual bool loaded() const noexcept = 0;

    virtual uint32_t getNumMeshVertices(unsigned int mesh) const noexcept = 0;

    virtual auto importArmature() -> std::vector<foeArmatureNode> = 0;
    virtual auto importAnimation(unsigned int animation) -> foeAnimation = 0;

    virtual void importMeshVertexData(unsigned int mesh,
                                      uint32_t componentCount,
                                      foeVertexComponent const *pComponents,
                                      glm::mat4 const &transform,
                                      float *pData) = 0;

    virtual auto importMeshVertexData(unsigned int mesh,
                                      uint32_t componentCount,
                                      foeVertexComponent const *pComponents,
                                      glm::mat4 const &transform) -> std::vector<float> = 0;

    virtual uint32_t getNumFaces(unsigned int mesh) const noexcept = 0;

    virtual void importMeshIndexData16(unsigned int mesh, uint16_t offset, uint16_t *pData) = 0;

    virtual auto importMeshIndexData16(unsigned int mesh, uint16_t offset)
        -> std::vector<uint16_t> = 0;

    virtual void importMeshIndexData32(unsigned int mesh, uint32_t offset, uint32_t *pData) = 0;

    virtual auto importMeshIndexData32(unsigned int mesh, uint32_t offset)
        -> std::vector<uint32_t> = 0;

    virtual auto getMeshVerticesByWeight(unsigned int mesh) -> std::vector<int> = 0;

    virtual auto getMeshMaxPerVertexWeights(unsigned int mesh) -> int = 0;

    virtual void importMeshVertexBoneWeights(unsigned int mesh,
                                             int maxPerVertexWeights,
                                             void *pData) = 0;

    virtual auto importMeshBones(unsigned int mesh) -> std::vector<foeMeshBone> = 0;
};

#endif // FOE_MODEL_IMPORTER_HPP
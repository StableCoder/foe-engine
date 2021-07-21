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

#include "assimp_mesh_loader.hpp"

#include "ai_glm_conversion.hpp"

#include <algorithm>

void importMeshVertexData(aiMesh const *pMesh,
                          uint32_t componentCount,
                          foeVertexComponent const *pComponents,
                          glm::mat4 const &transform,
                          float *pData) {
    constexpr glm::vec4 zero{0.f, 0.f, 0.f, 0.f};
    constexpr glm::vec4 white{1.f, 1.f, 1.f, 1.f};

    for (unsigned int v = 0; v < pMesh->mNumVertices; ++v) {
        int uvIndex = 0;
        int colourIndex = 0;
        glm::vec4 coord;

        for (unsigned int c = 0; c < componentCount; ++c) {
            switch (pComponents[c]) {
            case foeVertexComponent::Position:
                coord = glm::vec4{pMesh->mVertices[v].x, pMesh->mVertices[v].y,
                                  pMesh->mVertices[v].z, 0.f};
                coord = coord * transform;

                *pData++ = coord.x;
                *pData++ = coord.y;
                *pData++ = coord.z;
                break;

            case foeVertexComponent::Normal:
                coord = glm::vec4{pMesh->mNormals[v].x, pMesh->mNormals[v].y, pMesh->mNormals[v].z,
                                  0.f};
                coord = coord * transform;

                *pData++ = coord.x;
                *pData++ = coord.y;
                *pData++ = coord.z;
                break;

            case foeVertexComponent::UV:
                coord = (pMesh->HasTextureCoords(uvIndex))
                            ? coord = glm::vec4{pMesh->mTextureCoords[uvIndex][v].x,
                                                pMesh->mTextureCoords[uvIndex][v].y, 0.f, 0.f}
                            : zero;

                *pData++ = coord.x;
                *pData++ = coord.y;
                ++uvIndex;
                break;

            case foeVertexComponent::Colour:
                coord = (pMesh->HasVertexColors(colourIndex))
                            ? coord = glm::vec4{pMesh->mColors[colourIndex][v].r,
                                                pMesh->mColors[colourIndex][v].g,
                                                pMesh->mColors[colourIndex][v].b,
                                                pMesh->mColors[colourIndex][v].a}
                            : white;

                *pData++ = coord.r;
                *pData++ = coord.g;
                *pData++ = coord.b;
                *pData++ = coord.a;
                ++colourIndex;
                break;

            case foeVertexComponent::Tangent:
                if (pMesh->HasTangentsAndBitangents()) {
                    coord = glm::vec4{pMesh->mTangents[v].x, pMesh->mTangents[v].y,
                                      pMesh->mTangents[v].z, 0.f};
                    coord = coord * transform;
                    coord = glm::normalize(coord);
                } else {
                    coord = zero;
                }

                *pData++ = coord.x;
                *pData++ = coord.y;
                *pData++ = coord.z;
                break;

            case foeVertexComponent::Bitangent:
                if (pMesh->HasTangentsAndBitangents()) {
                    coord = glm::vec4{pMesh->mBitangents[v].x, pMesh->mBitangents[v].y,
                                      pMesh->mBitangents[v].z, 0.f};
                    coord = coord * transform;
                    coord = glm::normalize(coord);
                } else {
                    coord = zero;
                }

                *pData++ = coord.x;
                *pData++ = coord.y;
                *pData++ = coord.z;
                break;
            }
        }
    }
}

auto importMeshVertexData(aiMesh const *pMesh,
                          uint32_t componentCount,
                          foeVertexComponent const *pComponents,
                          glm::mat4 const &transform) -> std::vector<float> {
    std::vector<float> vertexData(
        pMesh->mNumVertices *
        (foeGetVertexComponentStride(componentCount, pComponents) / sizeof(float)));

    importMeshVertexData(pMesh, componentCount, pComponents, transform, vertexData.data());

    return vertexData;
}

void importMeshIndexData16(aiMesh const *pMesh, uint16_t offset, uint16_t *pData) {
    for (unsigned int f = 0; f < pMesh->mNumFaces; ++f) {
        *pData++ = offset + pMesh->mFaces[f].mIndices[0];
        *pData++ = offset + pMesh->mFaces[f].mIndices[1];
        *pData++ = offset + pMesh->mFaces[f].mIndices[2];
    }
}

auto importMeshIndexData16(aiMesh const *pMesh, uint16_t offset) -> std::vector<uint16_t> {
    std::vector<uint16_t> indexData(pMesh->mNumFaces * 3);

    importMeshIndexData16(pMesh, offset, indexData.data());

    return indexData;
}

void importMeshIndexData32(aiMesh const *pMesh, uint32_t offset, uint32_t *pData) {
    for (unsigned int f = 0; f < pMesh->mNumFaces; ++f) {
        *pData++ = offset + pMesh->mFaces[f].mIndices[0];
        *pData++ = offset + pMesh->mFaces[f].mIndices[1];
        *pData++ = offset + pMesh->mFaces[f].mIndices[2];
    }
}

auto importMeshIndexData32(aiMesh const *pMesh, uint32_t offset) -> std::vector<uint32_t> {
    std::vector<uint32_t> indexData(pMesh->mNumFaces * 3);

    importMeshIndexData32(pMesh, offset, indexData.data());

    return indexData;
}
namespace {

auto getMeshWeightsPerVertex(aiMesh const *pMesh) -> std::vector<int> {
    std::vector<int> weightsPerVertex;
    weightsPerVertex.resize(pMesh->mNumVertices);

    for (unsigned int b = 0; b < pMesh->mNumBones; ++b) {
        aiBone const *pBone = pMesh->mBones[b];

        for (unsigned int w = 0; w < pBone->mNumWeights; ++w) {
            ++weightsPerVertex[pBone->mWeights[w].mVertexId];
        }
    }

    return weightsPerVertex;
}

} // namespace

auto getMeshVerticesByWeight(aiMesh const *pMesh) -> std::vector<int> {
    auto weightsPerVertex = getMeshWeightsPerVertex(pMesh);

    std::vector<int> verticesByWeight;

    for (auto it : weightsPerVertex) {
        if (it >= verticesByWeight.size()) {
            verticesByWeight.resize(it + 1);
        }
        ++verticesByWeight[it];
    }

    return verticesByWeight;
}

auto getMeshMaxPerVertexWeights(aiMesh const *pMesh) -> int {
    auto weightsPerVertex = getMeshWeightsPerVertex(pMesh);

    int maxWeightsPerVertex = 0;

    for (auto it : weightsPerVertex) {
        maxWeightsPerVertex = std::max(maxWeightsPerVertex, it);
    }

    return maxWeightsPerVertex;
}

namespace {

using VertexBoneWeights = std::vector<std::tuple<float, uint32_t>>;

auto collapseWeights(VertexBoneWeights weights, int maxWeights) -> VertexBoneWeights {
    std::sort(weights.begin(), weights.end(),
              [](auto a, auto b) { return std::get<0>(a) > std::get<0>(b); });

    if (weights.size() <= maxWeights) {
        return weights;
    }

    weights.erase(weights.begin() + maxWeights, weights.end());

    float totalWeight = 0.;
    for (auto const &it : weights) {
        totalWeight += std::get<0>(it);
    }

    float multiplier = 1. / totalWeight;
    for (auto &it : weights) {
        std::get<0>(it) *= multiplier;
    }

    return weights;
}

} // namespace

void importMeshVertexBoneWeights(aiMesh const *pMesh, int maxPerVertexWeights, void *pData) {
    std::vector<VertexBoneWeights> vertexBoneWeights(pMesh->mNumVertices);

    for (unsigned int b = 0; b < pMesh->mNumBones; ++b) {
        aiBone const *pBone = pMesh->mBones[b];

        for (unsigned int w = 0; w < pBone->mNumWeights; ++w) {
            vertexBoneWeights[pBone->mWeights[w].mVertexId].emplace_back(pBone->mWeights[w].mWeight,
                                                                         b);
        }
    }

    uint8_t *pBytes = static_cast<uint8_t *>(pData);
    for (auto const &vertexWeights : vertexBoneWeights) {
        auto collapsedWeights = collapseWeights(vertexWeights, maxPerVertexWeights);

        auto *pIndex = reinterpret_cast<uint32_t *>(pBytes);
        auto *pWeight = reinterpret_cast<float *>(pBytes + sizeof(uint32_t) * maxPerVertexWeights);

        int written = 0;
        for (auto &it : collapsedWeights) {
            *pIndex++ = std::get<1>(it);
            *pWeight++ = std::get<0>(it);

            ++written;
        }

        while (written < maxPerVertexWeights) {
            *pIndex++ = 0;
            *pWeight++ = 0.f;

            ++written;
        }

        pBytes += maxPerVertexWeights * (sizeof(float) + sizeof(uint32_t));
    }
}

auto importMeshBones(aiMesh const *pMesh) -> std::vector<foeMeshBone> {
    std::vector<foeMeshBone> meshBones(pMesh->mNumBones);

    for (unsigned int b = 0; b < pMesh->mNumBones; ++b) {
        aiBone const *pBone = pMesh->mBones[b];

        meshBones[b] = foeMeshBone{
            .name = pBone->mName.C_Str(),
            .offsetMatrix = toGlmMat4(pBone->mOffsetMatrix),
        };
    }

    return meshBones;
}
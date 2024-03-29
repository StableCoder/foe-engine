// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/model/cube.hpp>

#include "model_log.hpp"

#include <array>

namespace {

constexpr size_t cNumCubeVertices = 36;

constexpr std::array<float, 108> cCubePosition{
    1.0,  -1.0, -1.0, -1.0, 1.0,  -1.0, 1.0,  1.0,  -1.0, -1.0, 1.0,  1.0,  1.0,  -1.0, 1.0,  1.0,
    1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  -1.0, -1.0, 1.0,  1.0,  -1.0, 1.0,  -1.0, 1.0,  -1.0, -1.0,
    -1.0, 1.0,  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0,  1.0,  -1.0, 1.0,  -1.0, 1.0,  1.0,  -1.0,
    -1.0, 1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0,  -1.0, -1.0,
    1.0,  1.0,  -1.0, -1.0, 1.0,  1.0,  -1.0, 1.0,  1.0,  1.0,  1.0,  1.0,  -1.0, 1.0,  1.0,  -1.0,
    -1.0, 1.0,  -1.0, 1.0,  -1.0, -1.0, 1.0,  -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0,
    -1.0, 1.0,  1.0,  1.0,  1.0,  -1.0, -1.0, 1.0,  -1.0, -1.0, 1.0,  1.0,
};

constexpr std::array<float, 108> cCubeNormal{
    0.0,  0.0, -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  1.0,  0.0,  0.0,  1.0,  0.0,
    0.0,  1.0, 1.0,  0.0, 0.0,  1.0,  0.0, 0.0,  1.0,  0.0, 0.0,  0.0,  -1.0, 0.0,  0.0,  -1.0,
    0.0,  0.0, -1.0, 0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0,  0.0,  1.0,  0.0,
    0.0,  1.0, 0.0,  0.0, 1.0,  0.0,  0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0,  0.0,  -1.0, 0.0,
    0.0,  1.0, 0.0,  0.0, 1.0,  0.0,  0.0, 1.0,  1.0,  0.0, 0.0,  1.0,  0.0,  0.0,  1.0,  0.0,
    0.0,  0.0, -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, -1.0, 0.0,  0.0,  -1.0, 0.0,  0.0,
    -1.0, 0.0, 0.0,  0.0, 1.0,  0.0,  0.0, 1.0,  0.0,  0.0, 1.0,  0.0,
};

constexpr std::array<float, 108> cCubeTangent{
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
};

constexpr std::array<float, 108> cCubeBitangent{
    -1.0, 0.0, 0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, -1.0, 0.0,  0.0, 1.0,  0.0,  0.0, 1.0,  0.0,  0.0, 1.0,  0.0,  0.0, 1.0,
    0.0,  0.0, 1.0, 0.0,  0.0,  1.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, 0.0,
    -1.0, 0.0, 0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, 1.0,  0.0,  0.0, 1.0,
    0.0,  0.0, 1.0, 0.0,  0.0,  1.0, 0.0,  0.0,  1.0, 0.0,  0.0,  1.0,
};

constexpr std::array<float, 72> cCubeUV{
    0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0,
    1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,
    1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,
};

template <typename T>
void cubeVertexDataTemplate(uint32_t componentCount,
                            foeVertexComponent const *pComponents,
                            T *pData) {

    for (size_t v = 0; v < cNumCubeVertices; ++v) {
        for (uint32_t c = 0; c < componentCount; ++c) {

            switch (pComponents[c]) {
            case foeVertexComponent::Position:
                *pData++ = cCubePosition[v * 3 + 0];
                *pData++ = cCubePosition[v * 3 + 1];
                *pData++ = cCubePosition[v * 3 + 2];
                break;

            case foeVertexComponent::Normal:
                *pData++ = cCubeNormal[v * 3 + 0];
                *pData++ = cCubeNormal[v * 3 + 1];
                *pData++ = cCubeNormal[v * 3 + 2];
                break;

            case foeVertexComponent::UV:
                *pData++ = cCubeUV[v * 2 + 0];
                *pData++ = cCubeUV[v * 2 + 1];
                break;

            case foeVertexComponent::Tangent:
                *pData++ = cCubeTangent[v * 3 + 0];
                *pData++ = cCubeTangent[v * 3 + 1];
                *pData++ = cCubeTangent[v * 3 + 2];
                break;

            case foeVertexComponent::Bitangent:
                *pData++ = cCubeBitangent[v * 3 + 0];
                *pData++ = cCubeBitangent[v * 3 + 1];
                *pData++ = cCubeBitangent[v * 3 + 2];
                break;

            case foeVertexComponent::Colour:
                FOE_LOG(Model, FOE_LOG_LEVEL_FATAL,
                        "Tried to generate cube vertex data with an unsupported colour component");
                std::abort();
            }
        }
    }
}

constexpr std::array<uint16_t, 36> cCubeIndices{
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
};

template <typename IndexType>
auto cubeIndexData(IndexType offset, IndexType *pData) {
    for (auto it : cCubeIndices) {
        *pData++ = it + offset;
    }
}

} // namespace

int foeModelCubeNumVertices() noexcept { return cNumCubeVertices; }

void foeModelCubeVertexData(uint32_t componentCount,
                            foeVertexComponent const *pComponents,
                            float *pData) {
    ::cubeVertexDataTemplate(componentCount, pComponents, pData);
}

int foeModelCubeNumIndices() noexcept { return cCubeIndices.size(); }

void foeModelCubeIndexData16(uint16_t offset, uint16_t *pData) {
    ::cubeIndexData<uint16_t>(offset, pData);
}

void foeModelCubeIndexData32(uint32_t offset, uint32_t *pData) {
    ::cubeIndexData<uint32_t>(offset, pData);
}
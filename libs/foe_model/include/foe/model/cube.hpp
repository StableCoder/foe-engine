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

#ifndef FOE_MODEL_CUBE_HPP
#define FOE_MODEL_CUBE_HPP

#include <foe/model/export.h>
#include <foe/model/vertex_component.hpp>

#include <cstdint>

/**
 * @brief Returns the number of vertices for the cube mesh
 * @return Number of unique vertices worth of data
 */
FOE_MODEL_EXPORT int foeModelCubeNumVertices() noexcept;

/**
 * @brief Copies requested data components into the provided buffer
 * @param componentCount Number of vertex components requested
 * @param pComponents Array of component types to copy into the provided buffer, contiguously
 * @param pData[out] Buffer to copy the requested data into. Must be minimum of
 * `foeModelCubeNumVertices() * foeGetVertexComponentStride(componentCount,pComponents) *
 * sizeof(float)` bytes.
 */
FOE_MODEL_EXPORT void foeModelCubeVertexData(uint32_t componentCount,
                                             foeVertexComponent const *pComponents,
                                             float *pData);

/**
 * @brief Returns the number of indices for the cube mesh
 * @return Number of indices
 */
FOE_MODEL_EXPORT int foeModelCubeNumIndices() noexcept;

/**
 * @brief Copies cube 16-bit index data into the provided buffer
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of `foeModelCubeNumIndices() *
 * sizeof(uint16_t)` bytes.
 */
FOE_MODEL_EXPORT void foeModelCubeIndexData16(uint16_t offset, uint16_t *pData);

/**
 * @brief Copies cube 32-bit index data into the provided buffer
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of `foeModelCubeNumIndices() *
 * sizeof(uint32_t)` bytes.
 */
FOE_MODEL_EXPORT void foeModelCubeIndexData32(uint32_t offset, uint32_t *pData);

#endif // FOE_MODEL_CUBE_HPP
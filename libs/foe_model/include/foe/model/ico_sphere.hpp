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

#ifndef FOE_MODEL_ICO_SPHERE_HPP
#define FOE_MODEL_ICO_SPHERE_HPP

#include <foe/model/export.h>
#include <foe/model/vertex_component.hpp>

#include <cstdint>

/**
 * @brief Returns the number of vertices for the cube mesh
 * @param recursion Number of times to subdivide the cube
 * @return Number of unique vertices worth of data
 * @note Each level of recursion increases the smoothness of the cube, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT int foeModelIcoSphereNumVertices(int recursion) noexcept;

/**
 * @brief Copies requested data components into the provided buffer
 * @param recursion Number of times to subdivide the cube
 * @param componentCount Number of vertex components requested
 * @param pComponents Array of component types to copy into the provided buffer, contiguously
 * @param pData[out] Buffer to copy the requested data into. Must be minimum of
 * `foeModelIcoSphereNumVertices() * foeGetVertexComponentStride(componentCount,pComponents) *
 * sizeof(float)` bytes.
 * @note Each level of recursion increases the smoothness of the cube, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT void foeModelIcoSphereVertexData(int recursion,
                                                  uint32_t componentCount,
                                                  foeVertexComponent const *pComponents,
                                                  float *pData);

/**
 * @brief Returns the number of indices for the cube mesh
 * @param recursion Number of times to subdivide the cube
 * @return Number of indices
 * @note Each level of recursion increases the smoothness of the cube, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT int foeModelIcoSphereNumIndices(int recursion) noexcept;

/**
 * @brief Copies cube 16-bit index data into the provided buffer
 * @param recursion Number of times to subdivide the cube
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of
 * `foeModelIcoSphereNumIndices() * sizeof(uint16_t)` bytes.
 * @note Each level of recursion increases the smoothness of the cube, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT void foeModelIcoSphereIndexData16(int recursion, uint16_t offset, uint16_t *pData);

/**
 * @brief Copies cube 32-bit index data into the provided buffer
 * @param recursion Number of times to subdivide the cube
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of
 * `foeModelIcoSphereNumIndices() * sizeof(uint32_t)` bytes.
 * @note Each level of recursion increases the smoothness of the cube, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT void foeModelIcoSphereIndexData32(int recursion, uint32_t offset, uint32_t *pData);

#endif // FOE_MODEL_ICO_SPHERE_HPP
// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_ICO_SPHERE_HPP
#define FOE_MODEL_ICO_SPHERE_HPP

#include <foe/model/export.h>
#include <foe/model/vertex_component.hpp>

#include <cstdint>

/**
 * @brief Returns the number of vertices and indices via return parameters
 * @param recursion Number of times to subdivide the sphere
 * @param pNumVertices[out] Number of vertices for the recursion level
 * @param pNumindices[out] Number of indices for the recursion level
 */
FOE_MODEL_EXPORT
void foeModelIcoSphereNums(int recursion, int *pNumVertices, int *pNumIndices);

/**
 * @brief Returns the number of vertices for the sphere mesh
 * @param recursion Number of times to subdivide the sphere
 * @return Number of unique vertices worth of data
 * @note Each level of recursion increases the smoothness of the sphere, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT
int foeModelIcoSphereNumVertices(int recursion) noexcept;

/**
 * @brief Copies requested data components into the provided buffer
 * @param recursion Number of times to subdivide the sphere
 * @param componentCount Number of vertex components requested
 * @param pComponents Array of component types to copy into the provided buffer, contiguously
 * @param pData[out] Buffer to copy the requested data into. Must be minimum of
 * `foeModelIcoSphereNumVertices() * foeGetVertexComponentStride(componentCount,pComponents) *
 * sizeof(float)` bytes.
 * @note Each level of recursion increases the smoothness of the sphere, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT
void foeModelIcoSphereVertexData(int recursion,
                                 uint32_t componentCount,
                                 foeVertexComponent const *pComponents,
                                 float *pData);

/**
 * @brief Returns the number of indices for the sphere mesh
 * @param recursion Number of times to subdivide the sphere
 * @return Number of indices
 * @note Each level of recursion increases the smoothness of the sphere, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT
int foeModelIcoSphereNumIndices(int recursion) noexcept;

/**
 * @brief Copies sphere 16-bit index data into the provided buffer
 * @param recursion Number of times to subdivide the sphere
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of
 * `foeModelIcoSphereNumIndices() * sizeof(uint16_t)` bytes.
 * @note Each level of recursion increases the smoothness of the sphere, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT
void foeModelIcoSphereIndexData16(int recursion, uint16_t offset, uint16_t *pData);

/**
 * @brief Copies sphere 32-bit index data into the provided buffer
 * @param recursion Number of times to subdivide the sphere
 * @param offset Offset to add to the returned data
 * @param pData[out] Buffer to copy indice data to. Must be minimum of
 * `foeModelIcoSphereNumIndices() * sizeof(uint32_t)` bytes.
 * @note Each level of recursion increases the smoothness of the sphere, at the expense of
 * exponential number of vertices.
 */
FOE_MODEL_EXPORT
void foeModelIcoSphereIndexData32(int recursion, uint32_t offset, uint32_t *pData);

#endif // FOE_MODEL_ICO_SPHERE_HPP
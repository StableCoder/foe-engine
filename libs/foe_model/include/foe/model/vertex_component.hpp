// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_VERTEX_COMPONENT_HPP
#define FOE_MODEL_VERTEX_COMPONENT_HPP

#include <foe/model/export.h>

#include <cstdint>

/// Different component types for vertex information
enum class foeVertexComponent {
    Position,
    Normal,
    UV,
    Colour,
    Tangent,
    Bitangent,
};

/** @brief Returns the memory footprint/stride of a particular vertex component
 * @param component Type to get the stride of
 * @return Size of the component, in bytes
 */
FOE_MODEL_EXPORT int foeGetVertexComponentSize(foeVertexComponent component);

/** @brief Returns the memory footprint/stride of a particular set vertex components together
 * @param componentCount Number of components in the array
 * @param pComponents Items to get the stride of
 * @return Size of the component set, in bytes
 */
FOE_MODEL_EXPORT int foeGetVertexComponentStride(uint32_t componentCount,
                                                 foeVertexComponent const *pComponents);

#endif // FOE_MODEL_VERTEX_COMPONENT_HPP
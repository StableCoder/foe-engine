// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/model/vertex_component.hpp>

int foeGetVertexComponentSize(foeVertexComponent component) {
    switch (component) {
    case foeVertexComponent::Colour:
        return sizeof(float) * 4;

    case foeVertexComponent::Position:
    case foeVertexComponent::Normal:
    case foeVertexComponent::Tangent:
    case foeVertexComponent::Bitangent:
        return sizeof(float) * 3;

    case foeVertexComponent::UV:
        return sizeof(float) * 2;
    }
}

int foeGetVertexComponentStride(uint32_t componentCount, foeVertexComponent const *pComponents) {
    int stride = 0;

    while (componentCount > 0) {
        stride += foeGetVertexComponentSize(*pComponents++);
        --componentCount;
    }

    return stride;
}
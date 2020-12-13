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
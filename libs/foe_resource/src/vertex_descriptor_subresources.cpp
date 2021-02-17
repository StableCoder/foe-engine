/*
    Copyright (C) 2021 George Cave.

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

#include <foe/resource/vertex_descriptor.hpp>

#include <foe/resource/shader.hpp>

foeVertexDescriptor::SubResources::~SubResources() { reset(); }

foeVertexDescriptor::SubResources::SubResources(SubResources &&other) :
    pVertex{std::move(other.pVertex)},
    pTessellationControl{std::move(other.pTessellationControl)},
    pTessellationEvaluation{std::move(other.pTessellationEvaluation)},
    pGeometry{std::move(other.pGeometry)} {
    other.pVertex = nullptr;
    other.pTessellationControl = nullptr;
    other.pTessellationEvaluation = nullptr;
    other.pGeometry = nullptr;
}

auto foeVertexDescriptor::SubResources::operator=(SubResources &&other) -> SubResources & {
    reset();

    pVertex = std::move(other.pVertex);
    pTessellationControl = std::move(other.pTessellationControl);
    pTessellationEvaluation = std::move(other.pTessellationEvaluation);
    pGeometry = std::move(other.pGeometry);

    other.pVertex = nullptr;
    other.pTessellationControl = nullptr;
    other.pTessellationEvaluation = nullptr;
    other.pGeometry = nullptr;

    return *this;
}

void foeVertexDescriptor::SubResources::reset() {
    if (pVertex != nullptr) {
        pVertex->decrementUseCount();
        pVertex->decrementRefCount();
    }
    pVertex = nullptr;

    if (pTessellationControl != nullptr) {
        pTessellationControl->decrementUseCount();
        pTessellationControl->decrementRefCount();
    }
    pTessellationControl = nullptr;

    if (pTessellationEvaluation != nullptr) {
        pTessellationEvaluation->decrementUseCount();
        pTessellationEvaluation->decrementRefCount();
    }
    pTessellationEvaluation = nullptr;

    if (pGeometry != nullptr) {
        pGeometry->decrementUseCount();
        pGeometry->decrementRefCount();
    }
    pGeometry = nullptr;
}
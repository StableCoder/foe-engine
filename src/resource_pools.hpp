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

#ifndef RESOURCE_POOLS_HPP
#define RESOURCE_POOLS_HPP

#include <foe/resource/armature_pool.hpp>
#include <foe/resource/image_pool.hpp>
#include <foe/resource/material_pool.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/resource/shader_pool.hpp>
#include <foe/resource/vertex_descriptor_pool.hpp>

struct ResourcePools {
    // Other Resources
    foeArmaturePool armature;

    // Graphics Resources
    foeShaderPool shader;
    foeVertexDescriptorPool vertexDescriptor;
    foeImagePool image;
    foeMaterialPool material;
    foeMeshPool mesh;
};

#include <foe/resource/armature_loader.hpp>
#include <foe/resource/image_loader.hpp>
#include <foe/resource/material_loader.hpp>
#include <foe/resource/mesh_loader.hpp>
#include <foe/resource/shader_loader.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>

struct ResourceLoaders {
    // Other Resources
    foeArmatureLoader armature;

    // Graphics Resources
    foeShaderLoader shader;
    foeVertexDescriptorLoader vertexDescriptor;
    foeImageLoader image;
    foeMaterialLoader material;
    foeMeshLoader mesh;
};

#endif // RESOURCE_POOLS_HPP
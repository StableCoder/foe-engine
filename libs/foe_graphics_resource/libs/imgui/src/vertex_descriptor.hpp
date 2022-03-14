/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef VERTEX_DESCRIPTOR_HPP
#define VERTEX_DESCRIPTOR_HPP

struct foeVertexDescriptor;
struct foeVertexDescriptorCreateInfo;

void imgui_foeVertexDescriptor(foeVertexDescriptor const *pResource);

void imgui_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pCreateInfo);

#endif // VERTEX_DESCRIPTOR_HPP
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

#ifndef FOE_GRAPHICS_MESH_HPP
#define FOE_GRAPHICS_MESH_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/session.hpp>
#include <foe/handle.h>

#include <cstdint>

FOE_DEFINE_HANDLE(foeGfxMesh)

FOE_GFX_EXPORT uint32_t foeGfxGetMeshIndices(foeGfxMesh mesh);

FOE_GFX_EXPORT void foeGfxDestroyMesh(foeGfxSession session, foeGfxMesh mesh);

#endif // FOE_GRAPHICS_MESH_HPP
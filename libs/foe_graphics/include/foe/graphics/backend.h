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

#ifndef FOE_GRAPHICS_BACKEND_H
#define FOE_GRAPHICS_BACKEND_H

#include <foe/graphics/export.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeGfxVersion {
    unsigned int major : 10;
    unsigned int minor : 10;
    unsigned int patch : 12;
};

#ifdef __cplusplus
static_assert(sizeof(struct foeGfxVersion) == 4,
              "foeGfxVersion should be 4 bytes in size (uint32_t)");
#else
_Static_assert(sizeof(struct foeGfxVersion) == 4,
               "foeGfxVersion should be 4 bytes in size (uint32_t)");
#endif

FOE_GFX_EXPORT char const *foeGfxBackendName();

FOE_GFX_EXPORT struct foeGfxVersion foeGfxBackendVersion();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_BACKEND_H
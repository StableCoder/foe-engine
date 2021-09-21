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

#include <foe/graphics/backend.h>

#define VULKAN_BACKEND_NAME "Vulkan"

#define VULKAN_BACKEND_MAJOR 0
#define VULKAN_BACKEND_MINOR 0
#define VULKAN_BACKEND_PATCH 0

char const *foeGfxBackendName() { return VULKAN_BACKEND_NAME; }

struct foeGfxVersion foeGfxBackendVersion() {
    struct foeGfxVersion version = {
        .major = VULKAN_BACKEND_MAJOR,
        .minor = VULKAN_BACKEND_MINOR,
        .patch = VULKAN_BACKEND_PATCH,
    };

    return version;
}
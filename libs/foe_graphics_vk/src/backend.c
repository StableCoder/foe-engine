// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
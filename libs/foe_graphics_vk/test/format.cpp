// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/format.h>

TEST_CASE("foeGfxVkIsDepthFormat") {
    CHECK_FALSE(foeGfxVkIsDepthFormat(VK_FORMAT_UNDEFINED));

    CHECK(foeGfxVkIsDepthFormat(VK_FORMAT_D16_UNORM));
    CHECK(foeGfxVkIsDepthFormat(VK_FORMAT_D32_SFLOAT));
    CHECK(foeGfxVkIsDepthFormat(VK_FORMAT_D16_UNORM_S8_UINT));
    CHECK(foeGfxVkIsDepthFormat(VK_FORMAT_D24_UNORM_S8_UINT));
    CHECK(foeGfxVkIsDepthFormat(VK_FORMAT_D32_SFLOAT_S8_UINT));
}

TEST_CASE("foeGfxVkIsDepthStencilFormat") {
    CHECK_FALSE(foeGfxVkIsDepthStencilFormat(VK_FORMAT_UNDEFINED));

    CHECK_FALSE(foeGfxVkIsDepthStencilFormat(VK_FORMAT_D16_UNORM));
    CHECK_FALSE(foeGfxVkIsDepthStencilFormat(VK_FORMAT_D32_SFLOAT));

    CHECK(foeGfxVkIsDepthStencilFormat(VK_FORMAT_D16_UNORM_S8_UINT));
    CHECK(foeGfxVkIsDepthStencilFormat(VK_FORMAT_D24_UNORM_S8_UINT));
    CHECK(foeGfxVkIsDepthStencilFormat(VK_FORMAT_D32_SFLOAT_S8_UINT));
}

TEST_CASE("foeGfxVkBytesPerPixel - Colour") {
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 4);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 4);

    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_R8G8B8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 3);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_B8G8R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 3);

    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 1);
}

TEST_CASE("foeGfxVkBytesPerPixel - Depth/Stencil") {
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_COLOR_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_ASPECT_COLOR_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_COLOR_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_COLOR_BIT) == 0);

    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT) == 2);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT) == 4);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT) == 2);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT) == 3);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT) == 4);

    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_STENCIL_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_STENCIL_BIT) == 0);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT) == 1);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT) == 1);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT) == 1);

    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) == 2);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) == 4);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D16_UNORM_S8_UINT,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) == 3);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D24_UNORM_S8_UINT,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) == 4);
    CHECK(foeGfxVkBytesPerPixel(VK_FORMAT_D32_SFLOAT_S8_UINT,
                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) == 5);
}
// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/cleanup.h>
#include <foe/graphics/vk/shader.h>

#include "create_test_session.hpp"
#include "fragment_shader.h"
#include "vertex_shader.h"

TEST_CASE("foeGfxShader(Vulkan)") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;

    result = createTestSession(&runtime, &session);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);
    REQUIRE(session != FOE_NULL_HANDLE);

    SECTION("Failure Cases") {
        SECTION("Zero-sized code") {
            foeGfxShader shader = FOE_NULL_HANDLE;
            foeGfxVkShaderCreateInfo ci = {};

            result = foeGfxVkCreateShader(session, &ci, 0, vertexShader, &shader);
            REQUIRE(result.value == FOE_GRAPHICS_VK_ERROR_NO_PROVIDED_SHADER_CODE);
            REQUIRE(shader == FOE_NULL_HANDLE);
        }
        SECTION("No code provided (NULL)") {
            foeGfxShader shader = FOE_NULL_HANDLE;
            foeGfxVkShaderCreateInfo ci = {};

            result = foeGfxVkCreateShader(session, &ci, sizeof(vertexShader) / 4, NULL, &shader);
            REQUIRE(result.value == FOE_GRAPHICS_VK_ERROR_NO_PROVIDED_SHADER_CODE);
            REQUIRE(shader == FOE_NULL_HANDLE);
        }
    }

    SECTION("Success Cases") {
        SECTION("Vertex") {
            foeGfxShader shader = FOE_NULL_HANDLE;
            foeGfxVkShaderCreateInfo ci = {};

            result =
                foeGfxVkCreateShader(session, &ci, sizeof(vertexShader) / 4, vertexShader, &shader);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(shader != FOE_NULL_HANDLE);

            cleanup_foeGfxVkShaderCreateInfo(&ci);

            foeGfxDestroyShader(session, shader);
        }
        SECTION("Fragment") {
            foeGfxShader shader = FOE_NULL_HANDLE;
            foeGfxVkShaderCreateInfo ci = {};

            result = foeGfxVkCreateShader(session, &ci, sizeof(fragmentShader) / 4, fragmentShader,
                                          &shader);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(shader != FOE_NULL_HANDLE);

            cleanup_foeGfxVkShaderCreateInfo(&ci);

            foeGfxDestroyShader(session, shader);
        }
    }

    foeGfxDestroySession(session);
    foeGfxDestroyRuntime(runtime);
}
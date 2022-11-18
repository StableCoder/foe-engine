// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/result.h>

#include "render_graph_image_jobs.hpp"

TEST_CASE("foeGfxVkRenderGraph") {
    foeResultSet result;
    foeGfxVkRenderGraph renderGraph = FOE_NULL_HANDLE;

    result = foeGfxVkCreateRenderGraph(&renderGraph);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(renderGraph != FOE_NULL_HANDLE);

    SECTION("Returns an error when attempting to execute without first being compiled") {
        result = foeGfxVkRenderGraphExecute(renderGraph, FOE_NULL_HANDLE, FOE_NULL_HANDLE);
        CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_NOT_COMPILED);
    }

    SECTION("Compiling a render graph with no jobs will returns conditional success") {
        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_GRAPHICS_VK_NO_JOBS_TO_COMPILE);
        result = foeGfxVkRenderGraphExecute(renderGraph, FOE_NULL_HANDLE, FOE_NULL_HANDLE);
        CHECK(result.value == FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);
    }

    SECTION("Compiling with only non-required jobs leaves no jobs to execute after compilation") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "non-required import image", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);

        CHECK_FALSE(foeGfxVkRenderGraphJobToExecute(importImageJob));

        result = foeGfxVkRenderGraphExecute(renderGraph, FOE_NULL_HANDLE, FOE_NULL_HANDLE);
        CHECK(result.value == FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);
    }

    SECTION("Compiling with a required job compiles successfully and can be executed") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "non-required import image", true, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
    }

    foeGfxVkDestroyRenderGraph(renderGraph);
}

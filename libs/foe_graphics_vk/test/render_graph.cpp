// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/result.h>

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

    foeGfxVkDestroyRenderGraph(renderGraph);
}

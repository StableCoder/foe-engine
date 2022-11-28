// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/result.h>

TEST_CASE("foeGfxVkRenderGraph - Basics") {
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

    SECTION("Resource creation") {
        foeGfxVkRenderGraphResource resource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphResourceCreateInfo resourceCI{
            .sType = FOE_NULL_HANDLE,
            .pName = "testResource",
        };

        SECTION("Resource name can be retrieved") {
            result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(resource != FOE_NULL_HANDLE);

            CHECK(std::string_view{foeGfxVkRenderGraphGetResourceName(resource)} == "testResource");
        }

        SECTION("Creating a mutable resource") {
            resourceCI.isMutable = true;

            result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(resource != FOE_NULL_HANDLE);

            CHECK(foeGfxVkRenderGraphGetResourceIsMutable(resource));
        }
        SECTION("Creating an immutable resource") {
            resourceCI.isMutable = false;

            result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(resource != FOE_NULL_HANDLE);

            CHECK_FALSE(foeGfxVkRenderGraphGetResourceIsMutable(resource));
        }

        SECTION("No resource data provided") {
            result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(resource != FOE_NULL_HANDLE);

            CHECK(foeGfxVkRenderGraphGetResourceData(resource) == nullptr);
        }
        SECTION("A user data pointer provided") {
            resourceCI.pResourceData = &resource;

            result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
            REQUIRE(result.value == FOE_SUCCESS);
            REQUIRE(resource != FOE_NULL_HANDLE);

            CHECK(foeGfxVkRenderGraphGetResourceData(resource) == (void *)&resource);
        }
    }

    SECTION("Attempting to use an immutable resource in read/write mode on a job fails") {
        foeGfxVkRenderGraphResource resource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphResourceCreateInfo resourceCI{
            .sType = FOE_NULL_HANDLE,
            .pName = "testResource",
            .isMutable = false,
        };

        result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &resource);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);

        foeGfxVkRenderGraphResourceState resourceState{
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = resource,
        };

        foeGfxVkRenderGraphJobInfo jobInfo{
            .resourceCount = 1,
            .pResources = &resourceState,
            .name = "testJob",
        };

        foeGfxVkRenderGraphJob job = FOE_NULL_HANDLE;
        result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &job);
        CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_IMMUTABLE_RESOURCE);
        CHECK(job == FOE_NULL_HANDLE);
    }

    foeGfxVkDestroyRenderGraph(renderGraph);
}

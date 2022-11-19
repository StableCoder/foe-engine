// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/result.h>

#include "render_graph_image_jobs.hpp"

#include <array>

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

    SECTION("Upstream of required jobs are executed") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        foeGfxVkRenderGraphJob imageUseJob = FOE_NULL_HANDLE;
        result = singleImageResourceJob(
            renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
            RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageUseJob != FOE_NULL_HANDLE);

        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
        CHECK(foeGfxVkRenderGraphJobToExecute(imageUseJob));
    }

    SECTION("Upstream of required jobs are executed, downstream are not") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        foeGfxVkRenderGraphJob requiredJob = FOE_NULL_HANDLE;
        foeGfxVkRenderGraphJob nonRequiredJob = FOE_NULL_HANDLE;

        result = singleImageResourceJob(
            renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
            RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &requiredJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(requiredJob != FOE_NULL_HANDLE);

        result = singleImageResourceJob(
            renderGraph, "downstreamNotRequiredImageUseJob", false, imageResource,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
            RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &requiredJob, &nonRequiredJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(nonRequiredJob != FOE_NULL_HANDLE);

        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
        CHECK(foeGfxVkRenderGraphJobToExecute(requiredJob));
        CHECK_FALSE(foeGfxVkRenderGraphJobToExecute(nonRequiredJob));
    }

    SECTION(
        "When a resource is in a one-to-many job relationship, those downstream jobs should be all "
        "read-only and have the same incoming/outgoing states") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        std::array<foeGfxVkRenderGraphJob, 2> imageUseJobs = {};

        SECTION("When the modes are read-only and layouts match") {
            result = singleImageResourceJob(
                renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "downstreamNotRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &imageUseJobs[0], &imageUseJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_SUCCESS);

            CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageUseJobs[0]));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageUseJobs[1]));
        }

        SECTION("When the modes are a mix of read-only and read-write") {
            result = singleImageResourceJob(
                renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "downstreamNotRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &importImageJob, &imageUseJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the modes are a all read-write") {
            result = singleImageResourceJob(
                renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &importImageJob, &imageUseJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "downstreamNotRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &importImageJob, &imageUseJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the incoming states differ") {
            result = singleImageResourceJob(
                renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "downstreamNotRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the outgoing states differ") {
            result = singleImageResourceJob(
                renderGraph, "downstreamRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "downstreamNotRequiredImageUseJob", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageUseJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }
    }

    SECTION(
        "When a resource is in a many-to-one job relationship, then there should be no issues") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        std::array<foeGfxVkRenderGraphJob, 2> imageUseManyJobs = {};
        foeGfxVkRenderGraphJob imageUseOneJob = FOE_NULL_HANDLE;

        // The many jobs stage
        result = singleImageResourceJob(
            renderGraph, "downstreamManyImageUseJob0", false, imageResource,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
            RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseManyJobs[0]);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageUseManyJobs[0] != FOE_NULL_HANDLE);

        result = singleImageResourceJob(
            renderGraph, "downstreamManyImageUseJob1", false, imageResource,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
            RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob, &imageUseManyJobs[1]);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageUseManyJobs[1] != FOE_NULL_HANDLE);

        // The one job stage
        result = singleImageResourceJob(
            renderGraph, "downstreamOneImageUseJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            imageUseManyJobs.size(), imageUseManyJobs.data(), &imageUseOneJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageUseOneJob != FOE_NULL_HANDLE);

        result = foeGfxVkRenderGraphCompile(renderGraph);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
        CHECK(foeGfxVkRenderGraphJobToExecute(imageUseManyJobs[0]));
        CHECK(foeGfxVkRenderGraphJobToExecute(imageUseManyJobs[1]));
        CHECK(foeGfxVkRenderGraphJobToExecute(imageUseOneJob));
    }

    SECTION(
        "When a resource is in a many-to-many job relationship, then there should be no issues") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        std::array<foeGfxVkRenderGraphJob, 2> imageFirstManyJobs = {};
        std::array<foeGfxVkRenderGraphJob, 2> imageSecondManyJobs = {};

        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, false, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        result = singleImageResourceJob(renderGraph, "firstManyImageUseJob0", false, imageResource,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                                        RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob,
                                        &imageFirstManyJobs[0]);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageFirstManyJobs[0] != FOE_NULL_HANDLE);

        result = singleImageResourceJob(renderGraph, "firstManyImageUseJob1", false, imageResource,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                                        RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &importImageJob,
                                        &imageFirstManyJobs[1]);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageFirstManyJobs[1] != FOE_NULL_HANDLE);

        SECTION("Downstream are all read-only and same incoming/outgoing resource state") {
            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob0", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob1", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_SUCCESS);

            CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageFirstManyJobs[0]));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageFirstManyJobs[1]));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageSecondManyJobs[0]));
            CHECK(foeGfxVkRenderGraphJobToExecute(imageSecondManyJobs[1]));
        }

        SECTION("When the modes are a mix of read-only and read-write") {
            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob0", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob1", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the modes are a all read-write") {
            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob0", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob1", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the incoming resource states differ") {
            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob0", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob1", true, imageResource,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }

        SECTION("When the outgoing resource states differ") {
            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob0", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[0]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[0] != FOE_NULL_HANDLE);

            result = singleImageResourceJob(
                renderGraph, "secondManyImageUseJob1", true, imageResource,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, imageFirstManyJobs.size(),
                imageFirstManyJobs.data(), &imageSecondManyJobs[1]);
            CHECK(result.value == FOE_SUCCESS);
            CHECK(imageSecondManyJobs[1] != FOE_NULL_HANDLE);

            result = foeGfxVkRenderGraphCompile(renderGraph);
            CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
        }
    }

    foeGfxVkDestroyRenderGraph(renderGraph);
}

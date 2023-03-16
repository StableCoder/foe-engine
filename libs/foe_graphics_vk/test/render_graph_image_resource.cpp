// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/result.h>

#include "render_graph_image_jobs.hpp"

#include <array>

TEST_CASE("foeGfxVkRenderGraph - Image Resource") {
    foeResultSet result;
    foeGfxVkRenderGraph renderGraph = FOE_NULL_HANDLE;

    result = foeGfxVkCreateRenderGraph(&renderGraph);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(renderGraph != FOE_NULL_HANDLE);

    SECTION("Compiling with only non-required jobs leaves no jobs to execute after compilation") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "non-required import image", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
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
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
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
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
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
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
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

    SECTION("one-to-one") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        foeGfxVkRenderGraphJob upstreamJob = FOE_NULL_HANDLE;
        foeGfxVkRenderGraphJob downstreamJob = FOE_NULL_HANDLE;

        SECTION("read/write to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }

        SECTION("read/write to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }

        SECTION("read-only to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }

        SECTION("read-only to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }
    }

    SECTION("many-to-one") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        std::array<foeGfxVkRenderGraphJob, 2> upstreamJobs = {FOE_NULL_HANDLE, FOE_NULL_HANDLE};
        foeGfxVkRenderGraphJob downstreamJob = FOE_NULL_HANDLE;

        SECTION("read/write to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read/write to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read-only to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }

        SECTION("read-only to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJob != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJob));
                }
            }
        }
    }

    SECTION("one-to-many") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        foeGfxVkRenderGraphJob upstreamJob = FOE_NULL_HANDLE;
        std::array<foeGfxVkRenderGraphJob, 2> downstreamJobs = {FOE_NULL_HANDLE, FOE_NULL_HANDLE};

        SECTION("read/write to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read/write to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }
        }

        SECTION("read-only to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1, &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read-only to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJob);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJob != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1, &upstreamJob, &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }
        }
    }

    SECTION("many-to-many") {
        foeGfxVkRenderGraphResource imageResource = FOE_NULL_HANDLE;

        foeGfxVkRenderGraphJob importImageJob;
        result = createImageJob(renderGraph, "importImageJob", false, "testImage",
                                VK_IMAGE_LAYOUT_UNDEFINED, true, &imageResource, &importImageJob);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(imageResource != FOE_NULL_HANDLE);
        CHECK(importImageJob != VK_NULL_HANDLE);

        std::array<foeGfxVkRenderGraphJob, 2> upstreamJobs = {FOE_NULL_HANDLE, FOE_NULL_HANDLE};
        std::array<foeGfxVkRenderGraphJob, 2> downstreamJobs = {FOE_NULL_HANDLE, FOE_NULL_HANDLE};

        SECTION("read/write to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read/write to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read-only to read/write") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("fails to compile due to multiple write jobs in parallel") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE);
                }
            }
        }

        SECTION("read-only to read-only") {
            SECTION("same layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
                    upstreamJobs.size(), upstreamJobs.data(), &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }

            SECTION("different layout") {
                result = singleImageResourceJob(
                    renderGraph, "upstreamJob1", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "upstreamJob2", true, imageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_UNDEFINED, RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, 1,
                    &importImageJob, &upstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(upstreamJobs[1] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob1", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[0]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[0] != FOE_NULL_HANDLE);

                result = singleImageResourceJob(
                    renderGraph, "downstreamJob2", true, imageResource,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY, upstreamJobs.size(), upstreamJobs.data(),
                    &downstreamJobs[1]);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(downstreamJobs[1] != FOE_NULL_HANDLE);

                SECTION("compiles") {
                    result = foeGfxVkRenderGraphCompile(renderGraph);
                    CHECK(result.value == FOE_SUCCESS);

                    CHECK(foeGfxVkRenderGraphJobToExecute(importImageJob));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(upstreamJobs[1]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[0]));
                    CHECK(foeGfxVkRenderGraphJobToExecute(downstreamJobs[1]));
                }
            }
        }
    }

    foeGfxVkDestroyRenderGraph(renderGraph);
}

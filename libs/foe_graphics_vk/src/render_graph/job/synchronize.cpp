// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/synchronize.hpp>

#include "../../result.h"

foeResultSet foeGfxVkSynchronizeJob(foeGfxVkRenderGraph renderGraph,
                                    char const *pJobName,
                                    bool required,
                                    VkFence fence,
                                    uint32_t upstreamJobCount,
                                    foeGfxVkRenderGraphJob *pUpstreamJobs,
                                    foeGfxVkRenderGraphJob *pJob) {
    foeGfxVkRenderGraphJobInfo jobInfo = {
        .name = pJobName,
        .required = required,
        .otherUpstreamJobCount = upstreamJobCount,
        .pOtherUpstreamJobs = pUpstreamJobs,
        .fence = fence,
    };

    return foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pJob);
}
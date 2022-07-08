// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WORST_RESOURCE_STATE_HPP
#define WORST_RESOURCE_STATE_HPP

#include <foe/resource/resource.h>

/**
 * Returns the 'worst' state in a given array of resource handles. The priority is this:
 * - Failed
 * - Unloaded
 * - Loaded
 */
foeResourceLoadState worstResourceLoadState(uint32_t resourceCount, foeResource *pResources);

#endif // WORST_RESOURCE_STATE_HPP
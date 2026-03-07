// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ENGINE_DETAIL_H
#define FOE_ENGINE_DETAIL_H

#ifdef __cplusplus
extern "C" {
#endif

/// Basic string macro representing the canonical name of the engine
#define FOE_ENGINE_NAME "FoE-Engine"

#define FOE_MAKE_VERSION(major, minor, patch)                                                      \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

/// 32-bit value representing the 'version' of the engine
#define FOE_ENGINE_VERSION FOE_MAKE_VERSION(0, 1, 0)

#ifdef __cplusplus
}
#endif

#endif // FOE_ENGINE_DETAIL_HPP
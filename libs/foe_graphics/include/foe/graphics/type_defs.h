// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_TYPE_DEFS_H
#define FOE_GRAPHICS_TYPE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FOE_GRAPHICS_MAX_BUFFERED_FRAMES = 3,
};

enum {
    MaxQueueFamilies = 8U,
    MaxQueuesPerFamily = 8U,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_TYPE_DEFS_H
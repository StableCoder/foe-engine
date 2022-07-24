// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_TYPE_DEFS_H
#define FOE_TYPE_DEFS_H

#include <stdint.h>

/**
 * This macro is used to help diferentiate different binaries/plugin's functionality set.
 *
 * For the C-interface to work, some enum values, such as 'structure types' need to be unique across
 * all disparate compilations, but this cannot be easily assured normally. This handy macro, when
 * given a value between 0 and 1.000.000 will return an ID which should be the basis for enum block
 *of values which should be considered reserved for that functionality to use.
 *
 * For example, if AUD_FUN calls FOE_PLUGIN_ID(1024), then AUD_FUN should should
 * consider itself to have free reign to use values 1.001.024.000 to 1.001.024.999. This AUD_FUN
 *value should then be passed when the functionality is registered, so that it can be determined if
 *this value is shared with any other set of loaded functionality, and be rejected if clashes could
 * occur.
 *
 * If functionality is rejected for this reason, then choosing a different value (there are 990.000
 * to choose from).
 *
 * @warning 0 is reserved for the main application and should not be used for non-application
 *functionality
 * @warning 1 - 9.999 is reserved for FoE-developed functionality and should not be used by others.
 **/
#define FOE_PLUGIN_ID(X) 1000000000 + (X * 1000)

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t foeStructureType;

typedef struct foeBaseInStructure {
    foeStructureType sType;
    struct foeBaseInStructure const *pNext;
} foeBaseInStructure;

typedef struct foeBaseOutStructure {
    foeStructureType sType;
    struct foeBaseOutStructure *pNext;
} foeBaseOutStructure;

#ifdef __cplusplus
}
#endif

#endif // FOE_TYPE_DEFS_H
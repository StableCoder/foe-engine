// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_RESOURCE_H
#define FOE_RESOURCE_RESOURCE_H

#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/resource/export.h>
#include <foe/resource/type_defs.h>
#include <foe/result.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeResourceFns foeResourceFns;

FOE_DEFINE_HANDLE(foeResource)

// This call is when raw resource data is being modified, for example when loading/unloading data
typedef void (*PFN_foeResourceDataModify)(
    void *, // Context
    void *  // Pointer to the raw data in the resource to be modified
);

/// @returns True if the data was moved/destroyed, otherwise false. If false, usually because of
/// mismatching resource iteration.
typedef bool (*PFN_foeResourceUnloadCall)(foeResource,              // Resource
                                          uint32_t,                 // Iteration
                                          void *,                   // Unload modify call context
                                          PFN_foeResourceDataModify // Data modify call
);

// To *always* be called at the end of a loading process, success or failure.
// Sets data appropriately and decrements reference count of both Resource and CreateInfo safely.
typedef void (*PFN_foeResourcePostLoad)(
    foeResource,               // Resource
    foeResultSet,              // ErrorCode
    void *,                    // Load modify call context
    PFN_foeResourceDataModify, // Data modify call
    void *,                    // Unload Context
    void (*)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool) // Unload Fn
);

typedef enum foeResourceLoadState {
    FOE_RESOURCE_LOAD_STATE_UNLOADED = 0,
    FOE_RESOURCE_LOAD_STATE_LOADED,
    FOE_RESOURCE_LOAD_STATE_FAILED,
} foeResourceLoadState;

FOE_RES_EXPORT char const *foeResourceLoadStateToString(foeResourceLoadState state);

FOE_RES_EXPORT foeResultSet foeCreateUndefinedResource(foeResourceID id,
                                                       foeResourceFns const *pResourceFns,
                                                       foeResource *pResource);

FOE_RES_EXPORT foeResultSet foeCreateLoadedResource(
    foeResourceID id,
    foeResourceType type,
    foeResourceFns const *pResourceFns,
    size_t size,
    void *pLoadDataContext,
    PFN_foeResourceDataModify loadDataFn,
    void *pUnloadDataContext,
    void (*pUnloadDataFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool),
    foeResource *pResource);

FOE_RES_EXPORT foeResultSet foeCreateResource(foeResourceID id,
                                              foeResourceType type,
                                              foeResourceFns const *pResourceFns,
                                              size_t size, // Must be >= to sizeof(foeResourceBase)
                                              foeResource *pResource);

FOE_RES_EXPORT foeResultSet foeResourceReplace(foeResource oldResource, foeResource newResource);
// Returned resource has it's reference already incremented
FOE_RES_EXPORT foeResource foeResourceGetReplacement(foeResource resource);

FOE_RES_EXPORT foeResourceID foeResourceGetID(foeResource resource);

FOE_RES_EXPORT foeResourceType foeResourceGetType(foeResource resource);
FOE_RES_EXPORT bool foeResourceHasType(foeResource resource, foeResourceType type);

FOE_RES_EXPORT int foeResourceGetRefCount(foeResource resource);
FOE_RES_EXPORT int foeResourceIncrementRefCount(foeResource resource);
FOE_RES_EXPORT int foeResourceDecrementRefCount(foeResource resource);

FOE_RES_EXPORT int foeResourceGetUseCount(foeResource resource);
FOE_RES_EXPORT int foeResourceIncrementUseCount(foeResource resource);
FOE_RES_EXPORT int foeResourceDecrementUseCount(foeResource resource);

FOE_RES_EXPORT bool foeResourceGetIsLoading(foeResource resource);
FOE_RES_EXPORT foeResourceLoadState foeResourceGetState(foeResource resource);

FOE_RES_EXPORT void const *foeResourceGetData(foeResource resource);
FOE_RES_EXPORT void const *foeResourceGetTypeData(foeResource resource, foeResourceType type);

FOE_RES_EXPORT foeResultSet foeResourceLoadData(foeResource resource);
FOE_RES_EXPORT void foeResourceUnloadData(foeResource resource, bool immediate);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESOURCE_H
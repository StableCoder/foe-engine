/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef FOE_RESOURCE_RESOURCE_H
#define FOE_RESOURCE_RESOURCE_H

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/resource/create_info.h>
#include <foe/resource/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeResourceFns foeResourceFns;

FOE_DEFINE_HANDLE(foeResource)

typedef int foeResourceType;

/// @returns True if the data was moved/destroyed, otherwise false. If false, usually because of
/// mismatching resource iteration.
typedef bool(PFN_foeResourceUnloadCall)(foeResource,             // Resource
                                        uint32_t,                // Iteration
                                        void *,                  // Destination
                                        void (*)(void *, void *) // Data handling call
);

typedef void(PFN_foeResourcePostLoad)(
    foeResource,                                                               // Resource
    foeErrorCode,                                                              // ErrorCode
    void *,                                                                    // Source
    void (*)(void *, void *),                                                  // Move Fn
    foeResourceCreateInfo,                                                     // CreateInfo
    void *,                                                                    // Unload Context
    void (*)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool) // Unload Fn
);

typedef enum foeResourceLoadState {
    Unloaded = 0,
    Loaded,
    Failed,
} foeResourceLoadState;

FOE_RES_EXPORT foeErrorCode foeCreateResource(foeResourceID id,
                                              foeResourceType type,
                                              foeResourceFns const *pResourceFns,
                                              size_t size,
                                              foeResource *pResource);

FOE_RES_EXPORT void foeDestroyResource(foeResource resource);

FOE_RES_EXPORT foeResourceID foeResourceGetID(foeResource resource);
FOE_RES_EXPORT foeResourceType foeResourceGetType(foeResource resource);

FOE_RES_EXPORT int foeResourceGetRefCount(foeResource resource);
FOE_RES_EXPORT int foeResourceIncrementRefCount(foeResource resource);
FOE_RES_EXPORT int foeResourceDecrementRefCount(foeResource resource);

FOE_RES_EXPORT int foeResourceGetUseCount(foeResource resource);
FOE_RES_EXPORT int foeResourceIncrementUseCount(foeResource resource);
FOE_RES_EXPORT int foeResourceDecrementUseCount(foeResource resource);

FOE_RES_EXPORT bool foeResourceGetIsLoading(foeResource resource);
FOE_RES_EXPORT foeResourceLoadState foeResourceGetState(foeResource resource);

FOE_RES_EXPORT void const *foeResourceGetData(foeResource resource);

FOE_RES_EXPORT void foeResourceImportCreateInfo(foeResource resource);
FOE_RES_EXPORT void foeResourceLoad(foeResource resource, bool refreshCreateInfo);
FOE_RES_EXPORT void foeResourceUnload(foeResource resource, bool immediate);

// @warning The handle has already been pre-incremented before being returned. The used is
// responsible for decrementing the reference count before disposing of it.
FOE_RES_EXPORT foeResourceCreateInfo foeResourceGetCreateInfo(foeResource resource);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESOURCE_H
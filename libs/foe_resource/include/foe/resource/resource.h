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

#include <foe/ecs/id.hpp>
#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/resource/export.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeResourceFns;

FOE_DEFINE_HANDLE(foeResource)

typedef int foeResourceType;

enum foeResourceState {
    Unloaded = 0,
    Loaded,
    Failed,
};

FOE_RES_EXPORT foeErrorCode foeCreateResource(foeResourceID id,
                                              foeResourceType type,
                                              foeResourceFns *pResourceFns,
                                              void (*pDestroyFn)(foeResourceType, void *),
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
FOE_RES_EXPORT foeResourceState foeResourceGetState(foeResource resource);

FOE_RES_EXPORT void *foeResourceGetData(foeResource resource);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESOURCE_H
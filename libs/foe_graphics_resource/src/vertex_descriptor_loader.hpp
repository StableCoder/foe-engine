// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VERTEX_DESCRIPTOR_LOADER_HPP
#define VERTEX_DESCRIPTOR_LOADER_HPP

#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/type_defs.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <mutex>
#include <vector>

class foeVertexDescriptorLoader {
  public:
    foeResultSet initialize(foeResourcePool resourcePool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();

    void gfxMaintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfo createInfo);
    static void load(void *pLoader,
                     foeResource resource,
                     foeResourceCreateInfo createInfo,
                     PFN_foeResourcePostLoad postLoadFn);

  private:
    static void unloadResource(void *pContext,
                               foeResource resource,
                               uint32_t resourceIteration,
                               PFN_foeResourceUnloadCall unloadCallFn,
                               bool immediateUnload);

    void load(foeResource resource,
              foeResourceCreateInfo createInfo,
              PFN_foeResourcePostLoad postLoadFn);

    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad postLoadFn;
        foeVertexDescriptor data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall unloadCallFn;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // VERTEX_DESCRIPTOR_LOADER_HPP
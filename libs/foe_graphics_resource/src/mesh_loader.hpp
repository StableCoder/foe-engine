// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_LOADER_HPP
#define MESH_LOADER_HPP

#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/upload_buffer.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/upload_request.h>
#include <foe/managed_memory.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/result.h>

#include <array>
#include <filesystem>
#include <functional>
#include <mutex>
#include <vector>

class foeMeshLoader {
  public:
    foeResultSet initialize(
        foeResourcePool resourcePool,
        std::function<foeResultSet(char const *, foeManagedMemory *)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

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
    std::function<foeResultSet(char const *, foeManagedMemory *)> mExternalFileSearchFn;

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    struct LoadData {
        foeResource resource;
        PFN_foeResourcePostLoad postLoadFn;
        foeMesh data;

        foeGfxUploadRequest uploadRequest;
        foeGfxUploadBuffer uploadBuffer;
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

    std::mutex mDestroySync;
    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeMesh>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1> mDataDestroyLists{};
};

#endif // MESH_LOADER_HPP
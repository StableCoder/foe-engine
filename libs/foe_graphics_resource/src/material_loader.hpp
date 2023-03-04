// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATERIAL_LOADER_HPP
#define MATERIAL_LOADER_HPP

#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/result.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/fragment_descriptor_pool.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <vulkan/vulkan.h>

#include <array>
#include <mutex>
#include <vector>

class foeMaterialLoader {
  public:
    foeResultSet initialize(foeResourcePool resourcePool);
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
                     PFN_foeResourcePostLoad *pPostLoadFn);

  private:
    VkResult createDescriptorSet(foeMaterial *pMaterialData);

    static void unloadResource(void *pContext,
                               foeResource resource,
                               uint32_t resourceIteration,
                               PFN_foeResourceUnloadCall *pUnloadCallFn,
                               bool immediateUnload);

    void load(foeResource resource,
              foeResourceCreateInfo createInfo,
              PFN_foeResourcePostLoad *pPostLoadFn);

    foeResourcePool mResourcePool{FOE_NULL_HANDLE};
    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    foeGfxVkFragmentDescriptorPool mGfxFragmentDescriptorPool{FOE_NULL_HANDLE};

    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeMaterial data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall *pUnloadCallFn;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;

    std::mutex mDestroySync;
    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeMaterial>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1> mDataDestroyLists{};
};

#endif // MATERIAL_LOADER_HPP
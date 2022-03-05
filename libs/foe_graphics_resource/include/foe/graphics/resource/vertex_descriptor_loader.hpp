/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/simulation/core/create_info.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct FOE_GFX_RES_EXPORT foeVertexDescriptorCreateInfo : public foeResourceCreateInfoBase {
    foeId vertexShader;
    foeId tessellationControlShader;
    foeId tessellationEvaluationShader;
    foeId geometryShader;
    VkPipelineVertexInputStateCreateInfo vertexInputSCI;
    std::vector<VkVertexInputBindingDescription> inputBindings;
    std::vector<VkVertexInputAttributeDescription> inputAttributes;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblySCI;
    VkPipelineTessellationStateCreateInfo tessellationSCI;
};

class foeShaderPool;

class FOE_GFX_RES_EXPORT foeVertexDescriptorLoader {
  public:
    std::error_code initialize(foeShaderPool *pShaderPool);
    void deinitialize();
    bool initialized() const noexcept;

    void gfxMaintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo);
    static void load(void *pLoader,
                     void *pResource,
                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                     void (*pPostLoadFn)(void *, std::error_code));

  private:
    static void unloadResource(void *pContext,
                               void *pResource,
                               uint32_t resourceIteration,
                               bool immediateUnload);

    void load(void *pResource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              void (*pPostLoadFn)(void *, std::error_code));

    foeShaderPool *mShaderPool{nullptr};

    struct LoadData {
        foeVertexDescriptor *pResource;
        void (*pPostLoadFn)(void *, std::error_code);
        foeVertexDescriptor::Data data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeVertexDescriptor *pResource;
        uint32_t iteration;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP
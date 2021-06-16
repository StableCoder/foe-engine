/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_RESOURCE_VERTEX_DESCRIPTOR_HPP
#define FOE_RESOURCE_VERTEX_DESCRIPTOR_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/vk/vertex_descriptor.hpp>
#include <foe/resource/create_info_base.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>

#include <atomic>
#include <memory>
#include <mutex>

class foeShader;
class foeVertexDescriptorLoader;

struct foeVertexDescriptorCreateInfo : public foeResourceCreateInfoBase {
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

class foeVertexDescriptor {
  public:
    FOE_RES_EXPORT foeVertexDescriptor(foeId id,
                                       void (*pLoadFn)(void *, void *, bool),
                                       void *pLoadContext);
    FOE_RES_EXPORT ~foeVertexDescriptor();

    FOE_RES_EXPORT foeId getID() const noexcept;
    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT void requestLoad();
    FOE_RES_EXPORT void requestUnload();

    FOE_RES_EXPORT foeShader *getVertexShader() const noexcept;
    FOE_RES_EXPORT foeShader *getTessellationControlShader() const noexcept;
    FOE_RES_EXPORT foeShader *getTessellationEvaluationShader() const noexcept;
    FOE_RES_EXPORT foeShader *getGeometryShader() const noexcept;

    FOE_RES_EXPORT foeGfxVertexDescriptor const *getGfxVertexDescriptor() const noexcept;

  private:
    friend foeVertexDescriptorLoader;

    struct SubResources {
        foeShader *pVertex{nullptr};
        foeShader *pTessellationControl{nullptr};
        foeShader *pTessellationEvaluation{nullptr};
        foeShader *pGeometry{nullptr};

        SubResources() = default;
        FOE_RES_EXPORT ~SubResources();

        SubResources(SubResources const &) = delete;
        SubResources &operator=(SubResources const &) = delete;

        SubResources(SubResources &&);
        SubResources &operator=(SubResources &&);

        void reset();
    };

    // General
    foeId id;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    void (*mpLoadFn)(void *, void *, bool);
    void *mpLoadContext;

    std::mutex dataWriteLock{};

    SubResources loading;

  public:
    std::unique_ptr<foeVertexDescriptorCreateInfo> createInfo;
    struct Data {
        SubResources subResources{};
        foeGfxVertexDescriptor gfxVertDescriptor;
    };
    Data data{};
};

#endif // FOE_RESOURCE_VERTEX_DESCRIPTOR_HPP
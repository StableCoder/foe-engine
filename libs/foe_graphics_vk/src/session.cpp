// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/session.h>

#include <foe/delimited_string.h>
#include <foe/graphics/vk/runtime.h>

#include "log.hpp"
#include "queue_family.hpp"
#include "render_pass_pool.hpp"
#include "result.h"
#include "runtime.h"
#include "session.hpp"
#include "vk_result.h"

#include <memory>

namespace {

/// @todo Replace all with std::popcount when integrated across all platforms
inline uint32_t popcount(uint32_t value) noexcept {
#ifdef __GNUC__
    return __builtin_popcountll(value);
#elif __APPLE__
    return __popcount(value);
#else // WIN32
    uint32_t ret = 0;
    for (int i = 0; i < sizeof(value) * 8; ++i) {
        if (value & 0x1) {
            ++ret;
        }
        value >>= 1;
    }

    return ret;
#endif
}

void createQueueFamily(VkDevice device,
                       VkQueueFlags flags,
                       uint32_t family,
                       uint32_t numQueues,
                       QueueFamily *pQueueFamily) {
    if (numQueues >= MaxQueuesPerFamily) {
        FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_FATAL,
                "There are {} Vulkan queue families, when the maximum compiled support is {}",
                numQueues, MaxQueuesPerFamily)
        std::abort();
    }

    pQueueFamily->flags = flags;
    pQueueFamily->family = family;
    pQueueFamily->numQueues = numQueues;

    for (uint32_t i = 0; i < numQueues; ++i) {
        vkGetDeviceQueue(device, family, i, &pQueueFamily->queue[i]);
    }
}

void foeGfxVkDestroySession(foeGfxVkSession *pSession) {
    // Internal Pools
    if (pSession->pipelinePool)
        foeGfxVkDestroyPipelinePool(pSession->pipelinePool);
    pSession->builtinDescriptorSets.deinitialize(pSession->device);
    pSession->descriptorSetLayoutPool.deinitialize();
    pSession->renderPassPool.deinitialize();

    // State
    delete[] pSession->pExtensionNames;
    delete[] pSession->pLayerNames;

    if (pSession->allocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(pSession->allocator);

    if (pSession->device != VK_NULL_HANDLE)
        vkDestroyDevice(pSession->device, nullptr);

    delete pSession;
}

/** @brief Merges source feature flags to the destination
 * @param pSrc is a pointer to the struct of the source set of features to set in the destination
 * @param pDst is a pointer to the destintion where flags will be set
 */
void mergeFeatureSet_VkPhysicalDeviceFeatures(VkPhysicalDeviceFeatures const *pSrc,
                                              VkPhysicalDeviceFeatures *pDst) {
    // Generated from gen_merge_feature_set.sh
    if (pSrc->robustBufferAccess != VK_FALSE)
        pDst->robustBufferAccess = VK_TRUE;
    if (pSrc->fullDrawIndexUint32 != VK_FALSE)
        pDst->fullDrawIndexUint32 = VK_TRUE;
    if (pSrc->imageCubeArray != VK_FALSE)
        pDst->imageCubeArray = VK_TRUE;
    if (pSrc->independentBlend != VK_FALSE)
        pDst->independentBlend = VK_TRUE;
    if (pSrc->geometryShader != VK_FALSE)
        pDst->geometryShader = VK_TRUE;
    if (pSrc->tessellationShader != VK_FALSE)
        pDst->tessellationShader = VK_TRUE;
    if (pSrc->sampleRateShading != VK_FALSE)
        pDst->sampleRateShading = VK_TRUE;
    if (pSrc->dualSrcBlend != VK_FALSE)
        pDst->dualSrcBlend = VK_TRUE;
    if (pSrc->logicOp != VK_FALSE)
        pDst->logicOp = VK_TRUE;
    if (pSrc->multiDrawIndirect != VK_FALSE)
        pDst->multiDrawIndirect = VK_TRUE;
    if (pSrc->drawIndirectFirstInstance != VK_FALSE)
        pDst->drawIndirectFirstInstance = VK_TRUE;
    if (pSrc->depthClamp != VK_FALSE)
        pDst->depthClamp = VK_TRUE;
    if (pSrc->depthBiasClamp != VK_FALSE)
        pDst->depthBiasClamp = VK_TRUE;
    if (pSrc->fillModeNonSolid != VK_FALSE)
        pDst->fillModeNonSolid = VK_TRUE;
    if (pSrc->depthBounds != VK_FALSE)
        pDst->depthBounds = VK_TRUE;
    if (pSrc->wideLines != VK_FALSE)
        pDst->wideLines = VK_TRUE;
    if (pSrc->largePoints != VK_FALSE)
        pDst->largePoints = VK_TRUE;
    if (pSrc->alphaToOne != VK_FALSE)
        pDst->alphaToOne = VK_TRUE;
    if (pSrc->multiViewport != VK_FALSE)
        pDst->multiViewport = VK_TRUE;
    if (pSrc->samplerAnisotropy != VK_FALSE)
        pDst->samplerAnisotropy = VK_TRUE;
    if (pSrc->textureCompressionETC2 != VK_FALSE)
        pDst->textureCompressionETC2 = VK_TRUE;
    if (pSrc->textureCompressionASTC_LDR != VK_FALSE)
        pDst->textureCompressionASTC_LDR = VK_TRUE;
    if (pSrc->textureCompressionBC != VK_FALSE)
        pDst->textureCompressionBC = VK_TRUE;
    if (pSrc->occlusionQueryPrecise != VK_FALSE)
        pDst->occlusionQueryPrecise = VK_TRUE;
    if (pSrc->pipelineStatisticsQuery != VK_FALSE)
        pDst->pipelineStatisticsQuery = VK_TRUE;
    if (pSrc->vertexPipelineStoresAndAtomics != VK_FALSE)
        pDst->vertexPipelineStoresAndAtomics = VK_TRUE;
    if (pSrc->fragmentStoresAndAtomics != VK_FALSE)
        pDst->fragmentStoresAndAtomics = VK_TRUE;
    if (pSrc->shaderTessellationAndGeometryPointSize != VK_FALSE)
        pDst->shaderTessellationAndGeometryPointSize = VK_TRUE;
    if (pSrc->shaderImageGatherExtended != VK_FALSE)
        pDst->shaderImageGatherExtended = VK_TRUE;
    if (pSrc->shaderStorageImageExtendedFormats != VK_FALSE)
        pDst->shaderStorageImageExtendedFormats = VK_TRUE;
    if (pSrc->shaderStorageImageMultisample != VK_FALSE)
        pDst->shaderStorageImageMultisample = VK_TRUE;
    if (pSrc->shaderStorageImageReadWithoutFormat != VK_FALSE)
        pDst->shaderStorageImageReadWithoutFormat = VK_TRUE;
    if (pSrc->shaderStorageImageWriteWithoutFormat != VK_FALSE)
        pDst->shaderStorageImageWriteWithoutFormat = VK_TRUE;
    if (pSrc->shaderUniformBufferArrayDynamicIndexing != VK_FALSE)
        pDst->shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderSampledImageArrayDynamicIndexing != VK_FALSE)
        pDst->shaderSampledImageArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderStorageBufferArrayDynamicIndexing != VK_FALSE)
        pDst->shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderStorageImageArrayDynamicIndexing != VK_FALSE)
        pDst->shaderStorageImageArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderClipDistance != VK_FALSE)
        pDst->shaderClipDistance = VK_TRUE;
    if (pSrc->shaderCullDistance != VK_FALSE)
        pDst->shaderCullDistance = VK_TRUE;
    if (pSrc->shaderFloat64 != VK_FALSE)
        pDst->shaderFloat64 = VK_TRUE;
    if (pSrc->shaderInt64 != VK_FALSE)
        pDst->shaderInt64 = VK_TRUE;
    if (pSrc->shaderInt16 != VK_FALSE)
        pDst->shaderInt16 = VK_TRUE;
    if (pSrc->shaderResourceResidency != VK_FALSE)
        pDst->shaderResourceResidency = VK_TRUE;
    if (pSrc->shaderResourceMinLod != VK_FALSE)
        pDst->shaderResourceMinLod = VK_TRUE;
    if (pSrc->sparseBinding != VK_FALSE)
        pDst->sparseBinding = VK_TRUE;
    if (pSrc->sparseResidencyBuffer != VK_FALSE)
        pDst->sparseResidencyBuffer = VK_TRUE;
    if (pSrc->sparseResidencyImage2D != VK_FALSE)
        pDst->sparseResidencyImage2D = VK_TRUE;
    if (pSrc->sparseResidencyImage3D != VK_FALSE)
        pDst->sparseResidencyImage3D = VK_TRUE;
    if (pSrc->sparseResidency2Samples != VK_FALSE)
        pDst->sparseResidency2Samples = VK_TRUE;
    if (pSrc->sparseResidency4Samples != VK_FALSE)
        pDst->sparseResidency4Samples = VK_TRUE;
    if (pSrc->sparseResidency8Samples != VK_FALSE)
        pDst->sparseResidency8Samples = VK_TRUE;
    if (pSrc->sparseResidency16Samples != VK_FALSE)
        pDst->sparseResidency16Samples = VK_TRUE;
    if (pSrc->sparseResidencyAliased != VK_FALSE)
        pDst->sparseResidencyAliased = VK_TRUE;
    if (pSrc->variableMultisampleRate != VK_FALSE)
        pDst->variableMultisampleRate = VK_TRUE;
    if (pSrc->inheritedQueries != VK_FALSE)
        pDst->inheritedQueries = VK_TRUE;
}

#ifdef VK_VERSION_1_1
/** @brief Merges source feature flags to the destination
 * @param pSrc is a pointer to the struct of the source set of features to set in the destination
 * @param pDst is a pointer to the destintion where flags will be set
 */
void mergeFeatureSet_VkPhysicalDeviceVulkan11Features(VkPhysicalDeviceVulkan11Features const *pSrc,
                                                      VkPhysicalDeviceVulkan11Features *pDst) {
    // Generated from gen_merge_feature_set.sh
    if (pSrc->storageBuffer16BitAccess != VK_FALSE)
        pDst->storageBuffer16BitAccess = VK_TRUE;
    if (pSrc->uniformAndStorageBuffer16BitAccess != VK_FALSE)
        pDst->uniformAndStorageBuffer16BitAccess = VK_TRUE;
    if (pSrc->storagePushConstant16 != VK_FALSE)
        pDst->storagePushConstant16 = VK_TRUE;
    if (pSrc->storageInputOutput16 != VK_FALSE)
        pDst->storageInputOutput16 = VK_TRUE;
    if (pSrc->multiview != VK_FALSE)
        pDst->multiview = VK_TRUE;
    if (pSrc->multiviewGeometryShader != VK_FALSE)
        pDst->multiviewGeometryShader = VK_TRUE;
    if (pSrc->multiviewTessellationShader != VK_FALSE)
        pDst->multiviewTessellationShader = VK_TRUE;
    if (pSrc->variablePointersStorageBuffer != VK_FALSE)
        pDst->variablePointersStorageBuffer = VK_TRUE;
    if (pSrc->variablePointers != VK_FALSE)
        pDst->variablePointers = VK_TRUE;
    if (pSrc->protectedMemory != VK_FALSE)
        pDst->protectedMemory = VK_TRUE;
    if (pSrc->samplerYcbcrConversion != VK_FALSE)
        pDst->samplerYcbcrConversion = VK_TRUE;
    if (pSrc->shaderDrawParameters != VK_FALSE)
        pDst->shaderDrawParameters = VK_TRUE;
}
#endif

#ifdef VK_VERSION_1_2
/** @brief Merges source feature flags to the destination
 * @param pSrc is a pointer to the struct of the source set of features to set in the destination
 * @param pDst is a pointer to the destintion where flags will be set
 */
void mergeFeatureSet_VkPhysicalDeviceVulkan12Features(VkPhysicalDeviceVulkan12Features const *pSrc,
                                                      VkPhysicalDeviceVulkan12Features *pDst) {
    // Generated from gen_merge_feature_set.sh
    if (pSrc->samplerMirrorClampToEdge != VK_FALSE)
        pDst->samplerMirrorClampToEdge = VK_TRUE;
    if (pSrc->drawIndirectCount != VK_FALSE)
        pDst->drawIndirectCount = VK_TRUE;
    if (pSrc->storageBuffer8BitAccess != VK_FALSE)
        pDst->storageBuffer8BitAccess = VK_TRUE;
    if (pSrc->uniformAndStorageBuffer8BitAccess != VK_FALSE)
        pDst->uniformAndStorageBuffer8BitAccess = VK_TRUE;
    if (pSrc->storagePushConstant8 != VK_FALSE)
        pDst->storagePushConstant8 = VK_TRUE;
    if (pSrc->shaderBufferInt64Atomics != VK_FALSE)
        pDst->shaderBufferInt64Atomics = VK_TRUE;
    if (pSrc->shaderSharedInt64Atomics != VK_FALSE)
        pDst->shaderSharedInt64Atomics = VK_TRUE;
    if (pSrc->shaderFloat16 != VK_FALSE)
        pDst->shaderFloat16 = VK_TRUE;
    if (pSrc->shaderInt8 != VK_FALSE)
        pDst->shaderInt8 = VK_TRUE;
    if (pSrc->descriptorIndexing != VK_FALSE)
        pDst->descriptorIndexing = VK_TRUE;
    if (pSrc->shaderInputAttachmentArrayDynamicIndexing != VK_FALSE)
        pDst->shaderInputAttachmentArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderUniformTexelBufferArrayDynamicIndexing != VK_FALSE)
        pDst->shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderStorageTexelBufferArrayDynamicIndexing != VK_FALSE)
        pDst->shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
    if (pSrc->shaderUniformBufferArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderSampledImageArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderStorageBufferArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderStorageImageArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderInputAttachmentArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderUniformTexelBufferArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->shaderStorageTexelBufferArrayNonUniformIndexing != VK_FALSE)
        pDst->shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE;
    if (pSrc->descriptorBindingUniformBufferUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingSampledImageUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingStorageImageUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingStorageBufferUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingUniformTexelBufferUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingStorageTexelBufferUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
    if (pSrc->descriptorBindingUpdateUnusedWhilePending != VK_FALSE)
        pDst->descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    if (pSrc->descriptorBindingPartiallyBound != VK_FALSE)
        pDst->descriptorBindingPartiallyBound = VK_TRUE;
    if (pSrc->descriptorBindingVariableDescriptorCount != VK_FALSE)
        pDst->descriptorBindingVariableDescriptorCount = VK_TRUE;
    if (pSrc->runtimeDescriptorArray != VK_FALSE)
        pDst->runtimeDescriptorArray = VK_TRUE;
    if (pSrc->samplerFilterMinmax != VK_FALSE)
        pDst->samplerFilterMinmax = VK_TRUE;
    if (pSrc->scalarBlockLayout != VK_FALSE)
        pDst->scalarBlockLayout = VK_TRUE;
    if (pSrc->imagelessFramebuffer != VK_FALSE)
        pDst->imagelessFramebuffer = VK_TRUE;
    if (pSrc->uniformBufferStandardLayout != VK_FALSE)
        pDst->uniformBufferStandardLayout = VK_TRUE;
    if (pSrc->shaderSubgroupExtendedTypes != VK_FALSE)
        pDst->shaderSubgroupExtendedTypes = VK_TRUE;
    if (pSrc->separateDepthStencilLayouts != VK_FALSE)
        pDst->separateDepthStencilLayouts = VK_TRUE;
    if (pSrc->hostQueryReset != VK_FALSE)
        pDst->hostQueryReset = VK_TRUE;
    if (pSrc->timelineSemaphore != VK_FALSE)
        pDst->timelineSemaphore = VK_TRUE;
    if (pSrc->bufferDeviceAddress != VK_FALSE)
        pDst->bufferDeviceAddress = VK_TRUE;
    if (pSrc->bufferDeviceAddressCaptureReplay != VK_FALSE)
        pDst->bufferDeviceAddressCaptureReplay = VK_TRUE;
    if (pSrc->bufferDeviceAddressMultiDevice != VK_FALSE)
        pDst->bufferDeviceAddressMultiDevice = VK_TRUE;
    if (pSrc->vulkanMemoryModel != VK_FALSE)
        pDst->vulkanMemoryModel = VK_TRUE;
    if (pSrc->vulkanMemoryModelDeviceScope != VK_FALSE)
        pDst->vulkanMemoryModelDeviceScope = VK_TRUE;
    if (pSrc->vulkanMemoryModelAvailabilityVisibilityChains != VK_FALSE)
        pDst->vulkanMemoryModelAvailabilityVisibilityChains = VK_TRUE;
    if (pSrc->shaderOutputViewportIndex != VK_FALSE)
        pDst->shaderOutputViewportIndex = VK_TRUE;
    if (pSrc->shaderOutputLayer != VK_FALSE)
        pDst->shaderOutputLayer = VK_TRUE;
    if (pSrc->subgroupBroadcastDynamicId != VK_FALSE)
        pDst->subgroupBroadcastDynamicId = VK_TRUE;
}
#endif

#ifdef VK_VERSION_1_3
/** @brief Merges source feature flags to the destination
 * @param pSrc is a pointer to the struct of the source set of features to set in the destination
 * @param pDst is a pointer to the destintion where flags will be set
 */
void mergeFeatureSet_VkPhysicalDeviceVulkan13Features(VkPhysicalDeviceVulkan13Features const *pSrc,
                                                      VkPhysicalDeviceVulkan13Features *pDst) {
    // Generated from gen_merge_feature_set.sh
    if (pSrc->robustImageAccess != VK_FALSE)
        pDst->robustImageAccess = VK_TRUE;
    if (pSrc->inlineUniformBlock != VK_FALSE)
        pDst->inlineUniformBlock = VK_TRUE;
    if (pSrc->descriptorBindingInlineUniformBlockUpdateAfterBind != VK_FALSE)
        pDst->descriptorBindingInlineUniformBlockUpdateAfterBind = VK_TRUE;
    if (pSrc->pipelineCreationCacheControl != VK_FALSE)
        pDst->pipelineCreationCacheControl = VK_TRUE;
    if (pSrc->privateData != VK_FALSE)
        pDst->privateData = VK_TRUE;
    if (pSrc->shaderDemoteToHelperInvocation != VK_FALSE)
        pDst->shaderDemoteToHelperInvocation = VK_TRUE;
    if (pSrc->shaderTerminateInvocation != VK_FALSE)
        pDst->shaderTerminateInvocation = VK_TRUE;
    if (pSrc->subgroupSizeControl != VK_FALSE)
        pDst->subgroupSizeControl = VK_TRUE;
    if (pSrc->computeFullSubgroups != VK_FALSE)
        pDst->computeFullSubgroups = VK_TRUE;
    if (pSrc->synchronization2 != VK_FALSE)
        pDst->synchronization2 = VK_TRUE;
    if (pSrc->textureCompressionASTC_HDR != VK_FALSE)
        pDst->textureCompressionASTC_HDR = VK_TRUE;
    if (pSrc->shaderZeroInitializeWorkgroupMemory != VK_FALSE)
        pDst->shaderZeroInitializeWorkgroupMemory = VK_TRUE;
    if (pSrc->dynamicRendering != VK_FALSE)
        pDst->dynamicRendering = VK_TRUE;
    if (pSrc->shaderIntegerDotProduct != VK_FALSE)
        pDst->shaderIntegerDotProduct = VK_TRUE;
    if (pSrc->maintenance4 != VK_FALSE)
        pDst->maintenance4 = VK_TRUE;
}
#endif

} // namespace

extern "C" foeResultSet foeGfxVkCreateSession(foeGfxRuntime runtime,
                                              VkPhysicalDevice vkPhysicalDevice,
                                              uint32_t layerCount,
                                              char const *const *ppLayerNames,
                                              uint32_t extensionCount,
                                              char const *const *ppExtensionNames,
                                              VkPhysicalDeviceFeatures const *pBasicFeatures,
                                              void const *pFeatures,
                                              foeGfxSession *pSession) {

    foeResultSet result = to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
    VkResult vkResult = VK_SUCCESS;

    auto *pNewSession = new (std::nothrow) foeGfxVkSession{
        .instance = reinterpret_cast<foeGfxVkRuntime *>(runtime)->instance,
        .physicalDevice = vkPhysicalDevice,
    };
    if (pNewSession == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    // Queues
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &pNewSession->numQueueFamilies,
                                             nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties(
        new VkQueueFamilyProperties[pNewSession->numQueueFamilies]);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &pNewSession->numQueueFamilies,
                                             queueFamilyProperties.get());

    uint32_t maxQueueCount = 0;
    std::unique_ptr<VkDeviceQueueCreateInfo[]> queueCI(
        new VkDeviceQueueCreateInfo[pNewSession->numQueueFamilies]);
    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        queueCI[i] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = queueFamilyProperties[i].queueCount,
        };
        maxQueueCount = std::max(maxQueueCount, queueFamilyProperties[i].queueCount);
    }

    std::unique_ptr<float[]> queuePriorities(new float[maxQueueCount]);
    std::fill_n(queuePriorities.get(), maxQueueCount, 0.f);

    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        queueCI[i].pQueuePriorities = queuePriorities.get();
    }

    // Layers / Extensions
    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    for (uint32_t i = 0; i < layerCount; ++i)
        layers.emplace_back(ppLayerNames[i]);
    for (uint32_t i = 0; i < extensionCount; ++i)
        extensions.emplace_back(ppExtensionNames[i]);

    // Go through feature sets and merge them together to one per struct
    if (pBasicFeatures == nullptr)
        pNewSession->features_1_0 = {};
    else
        pNewSession->features_1_0 = *pBasicFeatures;

    uint32_t vkApiVersion = foeGfxVkEnumerateApiVersion(runtime);

#ifdef VK_VERSION_1_1
    pNewSession->features_1_1 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    };
#endif
#ifdef VK_VERSION_1_2
    pNewSession->features_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    };

    if (vkApiVersion >= VK_MAKE_VERSION(1, 2, 0)) {
        pNewSession->features_1_1.pNext = &pNewSession->features_1_2;
    }
#endif
#ifdef VK_VERSION_1_3
    pNewSession->features_1_3 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    };

    if (vkApiVersion >= VK_MAKE_VERSION(1, 3, 0)) {
        pNewSession->features_1_2.pNext = &pNewSession->features_1_3;
    }
#endif

    while (pFeatures != nullptr) {
        VkBaseInStructure const *pIn = static_cast<VkBaseInStructure const *>(pFeatures);
        bool processed = false;

#ifdef VK_KHR_get_physical_device_properties2
        if (pIn->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
            mergeFeatureSet_VkPhysicalDeviceFeatures(
                &((VkPhysicalDeviceFeatures2 const *)pFeatures)->features,
                &pNewSession->features_1_0);
            processed = true;
        }
#endif
#ifdef VK_VERSION_1_1
        if (pIn->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES) {
            if (vkApiVersion >= VK_MAKE_VERSION(1, 1, 0)) {
                mergeFeatureSet_VkPhysicalDeviceVulkan11Features(
                    static_cast<VkPhysicalDeviceVulkan11Features const *>(pFeatures),
                    &pNewSession->features_1_1);
                processed = true;
            } else {
                result =
                    to_foeResult(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
                break;
            }
        }
#endif
#ifdef VK_VERSION_1_2
        if (pIn->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) {
            if (vkApiVersion >= VK_MAKE_VERSION(1, 2, 0)) {
                mergeFeatureSet_VkPhysicalDeviceVulkan12Features(
                    static_cast<VkPhysicalDeviceVulkan12Features const *>(pFeatures),
                    &pNewSession->features_1_2);
                processed = true;
            } else {
                result =
                    to_foeResult(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
                break;
            }
        }
#endif
#ifdef VK_VERSION_1_3
        if (pIn->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES) {
            if (vkApiVersion >= VK_MAKE_VERSION(1, 3, 0)) {
                mergeFeatureSet_VkPhysicalDeviceVulkan13Features(
                    static_cast<VkPhysicalDeviceVulkan13Features const *>(pFeatures),
                    &pNewSession->features_1_3);
                processed = true;
            } else {
                result =
                    to_foeResult(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
                break;
            }
        }
#endif

        if (!processed) {
            result = to_foeResult(FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT);
            break;
        }

        pFeatures = pIn->pNext;
    }
    if (result.value != FOE_SUCCESS)
        goto CREATE_FAILED;

    {
#ifdef VK_KHR_get_physical_device_properties2
        VkPhysicalDeviceFeatures2 features_1_0{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
#ifdef VK_VERSION_1_1
            .pNext = &pNewSession->features_1_1,
#endif
            .features = pNewSession->features_1_0,
        };
#endif

        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
#ifdef VK_KHR_get_physical_device_properties2
            .pNext = &features_1_0,
#endif
            .queueCreateInfoCount = pNewSession->numQueueFamilies,
            .pQueueCreateInfos = queueCI.get(),
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
#ifndef VK_VERSION_1_1
            .pEnabledFeatures = pBasicFeatures,
#endif
        };

        vkResult = vkCreateDevice(vkPhysicalDevice, &deviceCI, nullptr, &pNewSession->device);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    // Add layer/extension/feature state to session struct for future queries
    foeCreateDelimitedString(layers.size(), layers.data(), '\0', &pNewSession->layerNamesLength,
                             nullptr);
    if (pNewSession->layerNamesLength != 0) {
        pNewSession->pLayerNames = new char[pNewSession->layerNamesLength];
        foeCreateDelimitedString(layers.size(), layers.data(), '\0', &pNewSession->layerNamesLength,
                                 pNewSession->pLayerNames);
    }

    foeCreateDelimitedString(extensions.size(), extensions.data(), '\0',
                             &pNewSession->extensionNamesLength, nullptr);
    if (pNewSession->extensionNamesLength != 0) {
        pNewSession->pExtensionNames = new char[pNewSession->extensionNamesLength];
        foeCreateDelimitedString(extensions.size(), extensions.data(), '\0',
                                 &pNewSession->extensionNamesLength, pNewSession->pExtensionNames);
    }

    // Retrieve the queues
    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        createQueueFamily(pNewSession->device, queueFamilyProperties[i].queueFlags, i,
                          queueFamilyProperties[i].queueCount, &pNewSession->queueFamilies[i]);
    }

    { // Allocator
        VmaAllocatorCreateInfo allocatorCI{
            .physicalDevice = pNewSession->physicalDevice,
            .device = pNewSession->device,
        };

        vkResult = vmaCreateAllocator(&allocatorCI, &pNewSession->allocator);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    // Internal pools
    result = pNewSession->renderPassPool.initialize(session_to_handle(pNewSession));
    if (result.value != FOE_SUCCESS)
        goto CREATE_FAILED;

    vkResult = pNewSession->descriptorSetLayoutPool.initialize(pNewSession->device);
    if (vkResult != VK_SUCCESS)
        goto CREATE_FAILED;

    vkResult = pNewSession->builtinDescriptorSets.initialize(pNewSession->device,
                                                             &pNewSession->descriptorSetLayoutPool);
    if (vkResult != VK_SUCCESS)
        goto CREATE_FAILED;

    result = foeGfxVkCreatePipelinePool(session_to_handle(pNewSession), &pNewSession->pipelinePool);
    if (result.value != FOE_SUCCESS)
        goto CREATE_FAILED;

CREATE_FAILED:
    if (result.value == FOE_SUCCESS && vkResult != VK_SUCCESS)
        result = vk_to_foeResult(vkResult);

    if (result.value != FOE_SUCCESS) {
        foeGfxVkDestroySession(pNewSession);
    } else {
        *pSession = session_to_handle(pNewSession);
    }

    return result;
}

extern "C" foeResultSet foeGfxVkEnumerateSessionLayers(foeGfxSession session,
                                                       uint32_t *pLayerNamesLength,
                                                       char *pLayerNames) {
    auto *pSession = session_from_handle(session);

    return foeCopyDelimitedString(pSession->layerNamesLength, pSession->pLayerNames, '\0',
                                  pLayerNamesLength, pLayerNames)
               ? to_foeResult(FOE_GRAPHICS_VK_SUCCESS)
               : to_foeResult(FOE_GRAPHICS_VK_INCOMPLETE);
}

extern "C" foeResultSet foeGfxVkEnumerateSessionExtensions(foeGfxSession session,
                                                           uint32_t *pExtensionNamesLength,
                                                           char *pExtensionNames) {
    auto *pSession = session_from_handle(session);

    return foeCopyDelimitedString(pSession->extensionNamesLength, pSession->pExtensionNames, '\0',
                                  pExtensionNamesLength, pExtensionNames)
               ? to_foeResult(FOE_GRAPHICS_VK_SUCCESS)
               : to_foeResult(FOE_GRAPHICS_VK_INCOMPLETE);
}

extern "C" void foeGfxVkEnumerateSessionFeatures(foeGfxSession session,
                                                 VkPhysicalDeviceFeatures *pBasicFeatures,
                                                 void *pFeatures) {
    auto *pSession = session_from_handle(session);

    if (pBasicFeatures != nullptr) {
        *pBasicFeatures = pSession->features_1_0;
    }

    while (pFeatures != nullptr) {
        VkBaseOutStructure *pOut = static_cast<VkBaseOutStructure *>(pFeatures);
        VkBaseOutStructure *pNext = pOut->pNext;

#ifdef VK_KHR_get_physical_device_properties2
        if (pOut->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
            static_cast<VkPhysicalDeviceFeatures2 *>(pFeatures)->features = pSession->features_1_0;
        }
#endif
#ifdef VK_VERSION_1_1
        if (pOut->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES) {
            *static_cast<VkPhysicalDeviceVulkan11Features *>(pFeatures) = pSession->features_1_1;
        }
#endif
#ifdef VK_VERSION_1_2
        if (pOut->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) {
            *static_cast<VkPhysicalDeviceVulkan12Features *>(pFeatures) = pSession->features_1_2;
        }
#endif
#ifdef VK_VERSION_1_3
        if (pOut->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES) {
            *static_cast<VkPhysicalDeviceVulkan13Features *>(pFeatures) = pSession->features_1_3;
        }
#endif

        pOut->pNext = pNext;
        pFeatures = pOut->pNext;
    }
}

extern "C" uint32_t foeGfxVkGetNumQueueFamilies(foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    return pSession->numQueueFamilies;
}

extern "C" uint32_t foeGfxVkGetBestQueueFamily(foeGfxSession session, VkQueueFlags flags) {
    auto *pSession = session_from_handle(session);
    std::vector<std::pair<uint32_t, uint32_t>> compatibleQueueFamilies;

    for (uint32_t i = 0; i < MaxQueueFamilies; ++i) {
        if (pSession->queueFamilies[i].numQueues == 0)
            continue;

        if (pSession->queueFamilies[i].flags == flags) {
            return i;
        }
        if ((pSession->queueFamilies[i].flags & flags) == flags) {
            compatibleQueueFamilies.emplace_back(i, popcount(pSession->queueFamilies[i].flags));
        }
    }

    // Now iterate through the list, find the queue with the least number of extra flags
    if (compatibleQueueFamilies.empty()) {
        return std::numeric_limits<uint32_t>::max();
    }

    uint32_t leastFlagsIndex = 0;
    uint32_t leastFlags = compatibleQueueFamilies[0].second;

    for (uint32_t i = 1; i < compatibleQueueFamilies.size(); ++i) {
        if (compatibleQueueFamilies[i].second < leastFlags) {
            leastFlagsIndex = i;
            leastFlags = compatibleQueueFamilies[i].second;
        }
    }

    return compatibleQueueFamilies[leastFlagsIndex].first;
}

extern "C" VkInstance foeGfxVkGetInstance(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->instance;
}

extern "C" VkPhysicalDevice foeGfxVkGetPhysicalDevice(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->physicalDevice;
}

extern "C" VkDevice foeGfxVkGetDevice(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->device;
}

extern "C" VmaAllocator foeGfxVkGetAllocator(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->allocator;
}

extern "C" foeGfxVkQueueFamily getFirstQueue(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return queue_family_to_handle(&pSession->queueFamilies[0]);
}

extern "C" void foeGfxDestroySession(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    foeGfxVkDestroySession(pSession);
}

extern "C" void foeGfxWaitIdle(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    vkDeviceWaitIdle(pSession->device);
}

extern "C" VkDescriptorSet foeGfxVkGetDummySet(foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    return pSession->builtinDescriptorSets.getDummySet();
}

extern "C" VkDescriptorSetLayout foeGfxVkGetBuiltinLayout(
    foeGfxSession session, foeBuiltinDescriptorSetLayoutFlags builtinLayout) {
    auto *pSession = session_from_handle(session);

    return pSession->builtinDescriptorSets.getBuiltinLayout(builtinLayout);
}

extern "C" uint32_t foeGfxVkGetBuiltinSetLayoutIndex(
    foeGfxSession session, foeBuiltinDescriptorSetLayoutFlags builtinLayout) {
    auto *pSession = session_from_handle(session);

    return pSession->builtinDescriptorSets.getBuiltinSetLayoutIndex(builtinLayout);
}

extern "C" foeGfxVkRenderPassPool foeGfxVkGetRenderPassPool(foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    return render_pass_pool_to_handle(&pSession->renderPassPool);
}

extern "C" foeGfxVkFragmentDescriptorPool *foeGfxVkGetFragmentDescriptorPool(
    foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    return &pSession->fragmentDescriptorPool;
}

extern "C" VkSampleCountFlags foeGfxVkGetSupportedMSAA(foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pSession->physicalDevice, &properties);

    return properties.limits.framebufferColorSampleCounts &
           properties.limits.framebufferDepthSampleCounts;
}

extern "C" VkSampleCountFlags foeGfxVkGetMaxSupportedMSAA(foeGfxSession session) {
    VkSampleCountFlags counts = foeGfxVkGetSupportedMSAA(session);

    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}
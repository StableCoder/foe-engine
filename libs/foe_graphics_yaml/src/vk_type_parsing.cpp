/*
    Copyright (C) 2020 George Cave.

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

#include "vk_type_parsing.hpp"

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_value_serialization.hpp>
#include <vulkan/vulkan.h>

template <typename VkType>
bool yaml_read_optional_vk(std::string const &typeName,
                           std::string const &nodeName,
                           YAML::Node const &node,
                           VkType &data) {
    YAML::Node const &subNode = nodeName.empty() ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    if (!vk_parse<VkType>(typeName, subNode.as<std::string>(), &data)) {
        throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                               "' with value of: " + subNode.as<std::string>());
    }

    return true;
}

template <typename VkType>
bool yaml_read_required_vk(std::string const &typeName,
                           std::string const &nodeName,
                           YAML::Node const &node,
                           VkType &data) {
    if (!yaml_read_optional_vk(typeName, nodeName, node, data)) {
        throw foeYamlException(nodeName + " - Required node not found to parse as '" + typeName +
                               "'");
    }

    return true;
}

template <typename VkType>
bool yaml_write_required_vk(std::string const &typeName,
                            std::string const &nodeName,
                            VkType const &data,
                            YAML::Node &node) {
    std::string serialized;
    if (vk_serialize(typeName, data, &serialized)) {
        if (nodeName.empty()) {
            node = serialized;
        } else {
            node[nodeName] = serialized;
        }
    } else {
        throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName + "'");
    }

    return true;
}

template <typename VkType>
bool yaml_write_optional_vk(std::string const &typeName,
                            std::string const &nodeName,
                            VkType const &defaultData,
                            VkType const &data,
                            YAML::Node &node) {
    if (data == defaultData) {
        return false;
    }

    return yaml_write_required_vk(typeName, nodeName, data, node);
}

template bool yaml_read_required_vk<VkFlags>(std::string const &typeName,
                                             std::string const &nodeName,
                                             YAML::Node const &node,
                                             VkFlags &data);

template bool yaml_read_optional_vk<VkFlags>(std::string const &typeName,
                                             std::string const &nodeName,
                                             YAML::Node const &node,
                                             VkFlags &data);

template bool yaml_write_required_vk<VkFlags>(std::string const &typeName,
                                              std::string const &nodeName,
                                              VkFlags const &data,
                                              YAML::Node &node);

template bool yaml_write_optional_vk<VkFlags>(std::string const &typeName,
                                              std::string const &nodeName,
                                              VkFlags const &defaultValue,
                                              VkFlags const &data,
                                              YAML::Node &node);

#define INSTANTIATION(T)                                                                           \
                                                                                                   \
    template <>                                                                                    \
    bool yaml_read_optional<T>(std::string const &nodeName, YAML::Node const &node, T &data) {     \
        return yaml_read_optional_vk<T>(#T, nodeName, node, data);                                 \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    bool yaml_read_required<T>(std::string const &nodeName, YAML::Node const &node, T &data) {     \
        return yaml_read_required_vk<T>(#T, nodeName, node, data);                                 \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    bool yaml_write_required<T>(std::string const &nodeName, T const &data, YAML::Node &node) {    \
        return yaml_write_required_vk<T>(#T, nodeName, data, node);                                \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    bool yaml_write_optional<T>(std::string const &nodeName, T const &defaultData, T const &data,  \
                                YAML::Node &node) {                                                \
        return yaml_write_optional_vk<T>(#T, nodeName, defaultData, data, node);                   \
    }

INSTANTIATION(VkImageLayout)
INSTANTIATION(VkAttachmentLoadOp)
INSTANTIATION(VkAttachmentStoreOp)
INSTANTIATION(VkImageType)
INSTANTIATION(VkImageTiling)
INSTANTIATION(VkImageViewType)
INSTANTIATION(VkCommandBufferLevel)
INSTANTIATION(VkComponentSwizzle)
INSTANTIATION(VkDescriptorType)
INSTANTIATION(VkQueryType)
INSTANTIATION(VkBorderColor)
INSTANTIATION(VkPipelineBindPoint)
INSTANTIATION(VkPipelineCacheHeaderVersion)
INSTANTIATION(VkPipelineCacheCreateFlagBits)
INSTANTIATION(VkPrimitiveTopology)
INSTANTIATION(VkSharingMode)
INSTANTIATION(VkIndexType)
INSTANTIATION(VkFilter)
INSTANTIATION(VkSamplerMipmapMode)
INSTANTIATION(VkSamplerAddressMode)
INSTANTIATION(VkCompareOp)
INSTANTIATION(VkPolygonMode)
INSTANTIATION(VkFrontFace)
INSTANTIATION(VkBlendFactor)
INSTANTIATION(VkBlendOp)
INSTANTIATION(VkStencilOp)
INSTANTIATION(VkLogicOp)
INSTANTIATION(VkInternalAllocationType)
INSTANTIATION(VkSystemAllocationScope)
INSTANTIATION(VkPhysicalDeviceType)
INSTANTIATION(VkVertexInputRate)
INSTANTIATION(VkFormat)
INSTANTIATION(VkStructureType)
INSTANTIATION(VkSubpassContents)
INSTANTIATION(VkDynamicState)
INSTANTIATION(VkDescriptorUpdateTemplateType)
INSTANTIATION(VkObjectType)
INSTANTIATION(VkQueueFlagBits)
INSTANTIATION(VkCullModeFlagBits)
INSTANTIATION(VkRenderPassCreateFlagBits)
INSTANTIATION(VkDeviceQueueCreateFlagBits)
INSTANTIATION(VkMemoryPropertyFlagBits)
INSTANTIATION(VkMemoryHeapFlagBits)
INSTANTIATION(VkAccessFlagBits)
INSTANTIATION(VkBufferUsageFlagBits)
INSTANTIATION(VkBufferCreateFlagBits)
INSTANTIATION(VkShaderStageFlagBits)
INSTANTIATION(VkImageUsageFlagBits)
INSTANTIATION(VkImageCreateFlagBits)
INSTANTIATION(VkImageViewCreateFlagBits)
INSTANTIATION(VkSamplerCreateFlagBits)
INSTANTIATION(VkPipelineCreateFlagBits)
INSTANTIATION(VkPipelineShaderStageCreateFlagBits)
INSTANTIATION(VkColorComponentFlagBits)
INSTANTIATION(VkFenceCreateFlagBits)
// INSTANTIATION(VkSemaphoreCreateFlagBits)
INSTANTIATION(VkFormatFeatureFlagBits)
INSTANTIATION(VkQueryControlFlagBits)
INSTANTIATION(VkQueryResultFlagBits)
INSTANTIATION(VkCommandBufferUsageFlagBits)
INSTANTIATION(VkQueryPipelineStatisticFlagBits)
INSTANTIATION(VkImageAspectFlagBits)
INSTANTIATION(VkSparseImageFormatFlagBits)
INSTANTIATION(VkSparseMemoryBindFlagBits)
INSTANTIATION(VkPipelineStageFlagBits)
INSTANTIATION(VkCommandPoolCreateFlagBits)
INSTANTIATION(VkCommandPoolResetFlagBits)
INSTANTIATION(VkCommandBufferResetFlagBits)
INSTANTIATION(VkSampleCountFlagBits)
INSTANTIATION(VkAttachmentDescriptionFlagBits)
INSTANTIATION(VkStencilFaceFlagBits)
INSTANTIATION(VkDescriptorPoolCreateFlagBits)
INSTANTIATION(VkDependencyFlagBits)
INSTANTIATION(VkSemaphoreType)
INSTANTIATION(VkSemaphoreWaitFlagBits)
INSTANTIATION(VkPresentModeKHR)
INSTANTIATION(VkColorSpaceKHR)
INSTANTIATION(VkDisplayPlaneAlphaFlagBitsKHR)
INSTANTIATION(VkCompositeAlphaFlagBitsKHR)
INSTANTIATION(VkSurfaceTransformFlagBitsKHR)
// INSTANTIATION(VkSwapchainImageUsageFlagBitsANDROID)
INSTANTIATION(VkTimeDomainEXT)
INSTANTIATION(VkDebugReportFlagBitsEXT)
INSTANTIATION(VkDebugReportObjectTypeEXT)
INSTANTIATION(VkDeviceMemoryReportEventTypeEXT)
INSTANTIATION(VkRasterizationOrderAMD)
INSTANTIATION(VkExternalMemoryHandleTypeFlagBitsNV)
INSTANTIATION(VkExternalMemoryFeatureFlagBitsNV)
INSTANTIATION(VkValidationCheckEXT)
INSTANTIATION(VkValidationFeatureEnableEXT)
INSTANTIATION(VkValidationFeatureDisableEXT)
INSTANTIATION(VkSubgroupFeatureFlagBits)
INSTANTIATION(VkIndirectCommandsLayoutUsageFlagBitsNV)
INSTANTIATION(VkIndirectStateFlagBitsNV)
INSTANTIATION(VkIndirectCommandsTokenTypeNV)
INSTANTIATION(VkPrivateDataSlotCreateFlagBitsEXT)
INSTANTIATION(VkDescriptorSetLayoutCreateFlagBits)
INSTANTIATION(VkExternalMemoryHandleTypeFlagBits)
INSTANTIATION(VkExternalMemoryFeatureFlagBits)
INSTANTIATION(VkExternalSemaphoreHandleTypeFlagBits)
INSTANTIATION(VkExternalSemaphoreFeatureFlagBits)
INSTANTIATION(VkSemaphoreImportFlagBits)
INSTANTIATION(VkExternalFenceHandleTypeFlagBits)
INSTANTIATION(VkExternalFenceFeatureFlagBits)
INSTANTIATION(VkFenceImportFlagBits)
INSTANTIATION(VkSurfaceCounterFlagBitsEXT)
INSTANTIATION(VkDisplayPowerStateEXT)
INSTANTIATION(VkDeviceEventTypeEXT)
INSTANTIATION(VkDisplayEventTypeEXT)
INSTANTIATION(VkPeerMemoryFeatureFlagBits)
INSTANTIATION(VkMemoryAllocateFlagBits)
INSTANTIATION(VkDeviceGroupPresentModeFlagBitsKHR)
INSTANTIATION(VkSwapchainCreateFlagBitsKHR)
INSTANTIATION(VkViewportCoordinateSwizzleNV)
INSTANTIATION(VkDiscardRectangleModeEXT)
INSTANTIATION(VkSubpassDescriptionFlagBits)
INSTANTIATION(VkPointClippingBehavior)
INSTANTIATION(VkSamplerReductionMode)
INSTANTIATION(VkTessellationDomainOrigin)
INSTANTIATION(VkSamplerYcbcrModelConversion)
INSTANTIATION(VkSamplerYcbcrRange)
INSTANTIATION(VkChromaLocation)
INSTANTIATION(VkBlendOverlapEXT)
INSTANTIATION(VkCoverageModulationModeNV)
INSTANTIATION(VkCoverageReductionModeNV)
INSTANTIATION(VkValidationCacheHeaderVersionEXT)
INSTANTIATION(VkShaderInfoTypeAMD)
INSTANTIATION(VkQueueGlobalPriorityEXT)
INSTANTIATION(VkDebugUtilsMessageSeverityFlagBitsEXT)
INSTANTIATION(VkDebugUtilsMessageTypeFlagBitsEXT)
INSTANTIATION(VkConservativeRasterizationModeEXT)
INSTANTIATION(VkDescriptorBindingFlagBits)
INSTANTIATION(VkVendorId)
INSTANTIATION(VkDriverId)
INSTANTIATION(VkConditionalRenderingFlagBitsEXT)
INSTANTIATION(VkResolveModeFlagBits)
INSTANTIATION(VkShadingRatePaletteEntryNV)
INSTANTIATION(VkCoarseSampleOrderTypeNV)
INSTANTIATION(VkGeometryInstanceFlagBitsKHR)
INSTANTIATION(VkGeometryFlagBitsKHR)
INSTANTIATION(VkBuildAccelerationStructureFlagBitsKHR)
INSTANTIATION(VkCopyAccelerationStructureModeKHR)
INSTANTIATION(VkAccelerationStructureTypeKHR)
INSTANTIATION(VkGeometryTypeKHR)
INSTANTIATION(VkAccelerationStructureMemoryRequirementsTypeKHR)
// INSTANTIATION(VkAccelerationStructureBuildTypeKHR)
INSTANTIATION(VkRayTracingShaderGroupTypeKHR)
INSTANTIATION(VkMemoryOverallocationBehaviorAMD)
INSTANTIATION(VkFramebufferCreateFlagBits)
INSTANTIATION(VkScopeNV)
INSTANTIATION(VkComponentTypeNV)
INSTANTIATION(VkDeviceDiagnosticsConfigFlagBitsNV)
INSTANTIATION(VkPipelineCreationFeedbackFlagBitsEXT)
// INSTANTIATION(VkFullScreenExclusiveEXT)
INSTANTIATION(VkPerformanceCounterScopeKHR)
INSTANTIATION(VkPerformanceCounterUnitKHR)
INSTANTIATION(VkPerformanceCounterStorageKHR)
INSTANTIATION(VkPerformanceCounterDescriptionFlagBitsKHR)
INSTANTIATION(VkAcquireProfilingLockFlagBitsKHR)
INSTANTIATION(VkShaderCorePropertiesFlagBitsAMD)
INSTANTIATION(VkPerformanceConfigurationTypeINTEL)
INSTANTIATION(VkQueryPoolSamplingModeINTEL)
INSTANTIATION(VkPerformanceOverrideTypeINTEL)
INSTANTIATION(VkPerformanceParameterTypeINTEL)
INSTANTIATION(VkPerformanceValueTypeINTEL)
INSTANTIATION(VkShaderFloatControlsIndependence)
INSTANTIATION(VkPipelineExecutableStatisticFormatKHR)
INSTANTIATION(VkLineRasterizationModeEXT)
INSTANTIATION(VkShaderModuleCreateFlagBits)
INSTANTIATION(VkPipelineCompilerControlFlagBitsAMD)
INSTANTIATION(VkToolPurposeFlagBitsEXT)
INSTANTIATION(VkFragmentShadingRateCombinerOpKHR)

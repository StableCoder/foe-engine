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

#ifndef VULKAN_SETUP_HPP
#define VULKAN_SETUP_HPP

#include <openxr/openxr.h>
#include <vulkan/vulkan.h>

#include <string>
#include <tuple>
#include <vector>

auto determineVkInstanceEnvironment(XrInstance xrInstance,
                                    XrSystemId xrSystemId,
                                    bool enableWindowing,
                                    bool validation,
                                    bool debugLogging)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>>;

auto determineVkPhysicalDevice(VkInstance vkInstance,
                               XrInstance xrInstance,
                               XrSystemId xrSystemId,
                               VkSurfaceKHR vkSurface,
                               uint32_t explicitGpu,
                               bool forceXr) -> VkPhysicalDevice;

auto determineVkDeviceEnvironment(XrInstance xrInstance,
                                  XrSystemId xrSystemId,
                                  bool enableWindowing)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>>;

#endif // VULKAN_SETUP_HPP
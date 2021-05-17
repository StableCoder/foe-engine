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

#ifndef FOE_POSITION_COMPONENT_3D_HPP
#define FOE_POSITION_COMPONENT_3D_HPP

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct foePosition3d {
    glm::vec3 position;
    glm::quat orientation;

    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
};

#endif // FOE_POSITION_COMPONENT_3D_HPP
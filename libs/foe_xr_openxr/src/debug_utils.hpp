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

#ifndef FOE_XR_DEBUG_UTILS_HPP
#define FOE_XR_DEBUG_UTILS_HPP

#include <foe/xr/export.h>
#include <openxr/openxr.h>

FOE_XR_EXPORT XrResult
foeOpenXrCreateDebugUtilsMessenger(XrInstance instance, XrDebugUtilsMessengerEXT *pDebugMessenger);

FOE_XR_EXPORT XrResult foeOpenXrDestroyDebugUtilsMessenger(XrInstance instance,
                                                           XrDebugUtilsMessengerEXT debugMessenger);

#endif // FOE_XR_DEBUG_UTILS_HPP
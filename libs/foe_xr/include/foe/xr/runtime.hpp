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

#ifndef FOE_XR_RUNTIME_HPP
#define FOE_XR_RUNTIME_HPP

#include <foe/xr/export.h>
#include <openxr/openxr.h>

#include <string>
#include <system_error>
#include <vector>

struct FOE_XR_EXPORT foeXrRuntime {
    std::error_code createRuntime(char const *appName,
                                  uint32_t appVersion,
                                  std::vector<std::string> const &apiLayers,
                                  std::vector<std::string> const &extensions,
                                  bool debugLogging);
    void destroyRuntime();

    std::error_code pollEvent(XrEventDataBuffer &event);

    XrInstance instance{XR_NULL_HANDLE};
    XrDebugUtilsMessengerEXT debugMessenger{XR_NULL_HANDLE};
};

#endif // FOE_XR_RUNTIME_HPP
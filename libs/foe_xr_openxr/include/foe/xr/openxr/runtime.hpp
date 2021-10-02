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

#ifndef FOE_XR_OPENXR_RUNTIME_HPP
#define FOE_XR_OPENXR_RUNTIME_HPP

#include <foe/xr/runtime.hpp>
#include <openxr/openxr.h>

#include <string>
#include <system_error>
#include <vector>

FOE_XR_EXPORT std::error_code foeXrOpenCreateRuntime(char const *appName,
                                                     uint32_t appVersion,
                                                     std::vector<std::string> layers,
                                                     std::vector<std::string> extensions,
                                                     bool validation,
                                                     bool debugLogging,
                                                     foeXrRuntime *pRuntime);

#include <openxr/openxr.h>

FOE_XR_EXPORT XrInstance foeXrOpenGetInstance(foeXrRuntime runtime);

#endif // FOE_XR_OPENXR_RUNTIME_HPP
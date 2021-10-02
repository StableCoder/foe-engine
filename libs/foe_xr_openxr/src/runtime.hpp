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

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <foe/xr/runtime.hpp>
#include <openxr/openxr.h>

#include <mutex>
#include <vector>

struct foeXrSession;

struct foeXrOpenRuntime {
    std::mutex sync;

    XrInstance instance{XR_NULL_HANDLE};
    XrDebugUtilsMessengerEXT debugMessenger{XR_NULL_HANDLE};

    std::vector<foeXrSession *> sessions;
};

FOE_DEFINE_HANDLE_CASTS(runtime, foeXrOpenRuntime, foeXrRuntime)

void foeXrOpenAddSessionToRuntime(foeXrOpenRuntime *pRuntime, foeXrSession *pSession);

void foeXrOpenRemoveSessionFromRuntime(foeXrOpenRuntime *pRuntime, foeXrSession *pSession);

#endif // RUNTIME_HPP
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

#ifndef FOE_XR_SESSION_HPP
#define FOE_XR_SESSION_HPP

#include <foe/xr/export.h>
#include <foe/xr/runtime.hpp>
#include <openxr/openxr.h>

#include <system_error>
#include <vector>

struct FOE_XR_EXPORT foeXrSession {
    std::error_code createSession(foeXrRuntime runtime,
                                  XrSystemId systemId,
                                  XrViewConfigurationType configType,
                                  void const *pGraphicsBinding);
    void destroySession();

    std::error_code beginSession();
    std::error_code requestExitSession();
    std::error_code endSession();

    // From the runtime
    foeXrRuntime runtime;

    // For the session
    XrSystemId systemId;
    XrSession session;
    XrSessionState state;
    XrViewConfigurationType type;
    XrSpace space;
};

#endif // FOE_XR_SESSION_HPP
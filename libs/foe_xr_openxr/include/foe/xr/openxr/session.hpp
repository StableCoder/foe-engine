/*
    Copyright (C) 2020-2022 George Cave.

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

#ifndef FOE_XR_OPENXR_SESSION_HPP
#define FOE_XR_OPENXR_SESSION_HPP

#include <foe/error_code.h>
#include <foe/xr/export.h>
#include <foe/xr/runtime.h>
#include <openxr/openxr.h>

struct FOE_XR_EXPORT foeOpenXrSession {
    foeResult createSession(foeXrRuntime runtime,
                            XrSystemId systemId,
                            XrViewConfigurationType configType,
                            void const *pGraphicsBinding);
    void destroySession();

    foeResult beginSession();
    foeResult requestExitSession();
    foeResult endSession();

    // From the runtime
    foeXrRuntime runtime;

    // For the session
    XrSystemId systemId;
    XrSession session;
    XrSessionState state;
    /// This bool determines if we should be doing the wait/begin/end frame calls
    bool active{false};
    XrViewConfigurationType type;
    XrSpace space;
};

#endif // FOE_XR_OPENXR_SESSION_HPP
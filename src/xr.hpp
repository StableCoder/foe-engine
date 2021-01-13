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

#ifndef XR_HPP
#define XR_HPP

#include <foe/graphics/session.hpp>
#include <foe/xr/runtime.hpp>
#include <foe/xr/session.hpp>

#include <system_error>

std::error_code createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime);

std::error_code createXrSession(foeXrRuntime runtime,
                                foeGfxSession gfxSession,
                                foeXrSession *pSession);

#endif // XR_HPP
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

#ifndef FOE_XR_RUNTIME_H
#define FOE_XR_RUNTIME_H

#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Handle definition for foeXrRuntime
 *
 * Not all platforms support XR, espeically the Apple ecosystem. However, we still need the XR
 * runtime type to pass through to some other systems, notably Graphics.
 *
 * As such, this handle type can be used for passing around and compiling on non-XR supported
 * systems, without having to have two separate ABIs for other libraries, but to limit dummy stubs
 * or fancy export work, no functions are actually exported by this library.
 */
FOE_DEFINE_HANDLE(foeXrRuntime)

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_RUNTIME_H
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

#ifndef FOE_GRAPHICS_VK_ERROR_CODE_H
#define FOE_GRAPHICS_VK_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeGraphicsVkResult {
    FOE_GRAPHICS_VK_SUCCESS = 0,
    // RenderTarget
    FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_ERROR_CODE_H
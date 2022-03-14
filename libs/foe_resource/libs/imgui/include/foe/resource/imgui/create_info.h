/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_RESOURCE_IMGUI_CREATE_INFO_H
#define FOE_RESOURCE_IMGUI_CREATE_INFO_H

#include <foe/resource/create_info.h>
#include <foe/resource/imgui/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_RES_IMGUI_EXPORT void imgui_foeResourceCreateInfo(foeResourceCreateInfo createInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_IMGUI_CREATE_INFO_H
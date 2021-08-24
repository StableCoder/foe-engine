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

#ifndef FOE_WSI_ERROR_CODE_H
#define FOE_WSI_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeWsiResult {
    FOE_WSI_SUCCESS = 0,
    FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND,
    FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW,
    FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_ERROR_CODE_H
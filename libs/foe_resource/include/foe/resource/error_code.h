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

#ifndef FOE_RESOURCE_ERROR_CODE_H
#define FOE_RESOURCE_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeResourceResult {
    FOE_RESOURCE_SUCCESS = 0,
    FOE_RESOURCE_ERROR_ALREADY_INITIALIZED,
    FOE_RESOURCE_ERROR_IMPORT_FAILED,
    FOE_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_ERROR_CODE_H
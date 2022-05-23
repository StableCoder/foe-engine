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

#ifndef RESULT_H
#define RESULT_H

#include <foe/wsi/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResult to_foeResult(foeWsiResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeWsiResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H
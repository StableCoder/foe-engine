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

#ifndef FOE_ENGINE_DETAIL_H
#define FOE_ENGINE_DETAIL_H

#define FOE_ENGINE_NAME "FoE-Engine"

#define FOE_MAKE_VERSION(major, minor, patch)                                                      \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define FOE_ENGINE_VERSION FOE_MAKE_VERSION(0, 1, 0)

#endif // FOE_ENGINE_DETAIL_HPP
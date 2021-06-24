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

#ifndef FOE_RESOURCE_LOADER_BASE_HPP
#define FOE_RESOURCE_LOADER_BASE_HPP

struct foeResourceLoaderBase {
    virtual ~foeResourceLoaderBase() {}

    /**
     * @brief Performs any required 'maintenance' on related resource requests
     *
     * The 'maintenace' phase is supposed to be a section of time during the overall loop where the
     * referenced resources aren't being used as data sources for other systems, and thus can be
     * safely modified without causing issues in unrelated systems, and allows for far less
     * contention when reading/writing to resources.
     */
    virtual void maintenance() {}
};

#endif // FOE_RESOURCE_LOADER_BASE_HPP
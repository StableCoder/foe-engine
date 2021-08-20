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

#include "thread_pool_error_code.hpp"

namespace {

struct foeThreadPoolErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeThreadPoolErrCategory::name() const noexcept { return "foeSplitThreadResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeThreadPoolErrCategory::message(int ev) const {
    switch (static_cast<foeSplitThreadResult>(ev)) {
        RESULT_CASE(FOE_THREAD_POOL_SUCCESS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ALREADY_STARTED)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_NOT_STARTED)

    default:
        if (ev > 0)
            return "(unrecognized positive foeSplitThreadResult value)";
        else
            return "(unrecognized negative foeSplitThreadResult value)";
    }
}

const foeThreadPoolErrCategory threadPoolErrCategory{};

} // namespace

std::error_code make_error_code(foeSplitThreadResult e) {
    return {static_cast<int>(e), threadPoolErrCategory};
}
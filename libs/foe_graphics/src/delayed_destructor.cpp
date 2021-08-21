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

#include <foe/graphics/delayed_destructor.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace {

struct foeGfxDelayedDestructorImpl {
    foeGfxSession const session;

    std::atomic_uint currentDelay;
    std::mutex sync;
    std::vector<std::vector<foeGfxDelayedDestructorFn>> fnLists;
    std::vector<std::vector<foeGfxDelayedDestructorFn>>::iterator currentList;
};

FOE_DEFINE_HANDLE_CASTS(delayed_destructor, foeGfxDelayedDestructorImpl, foeGfxDelayedDestructor)

} // namespace

auto foeGfxCreateDelayedDestructor(foeGfxSession session,
                                   uint32_t initialDelay,
                                   foeGfxDelayedDestructor *pDelayedDestructor) -> std::error_code {

    auto *pNewDelayedDestructor = new foeGfxDelayedDestructorImpl{
        .session = session,
        .currentDelay = initialDelay,
        .fnLists =
            std::vector<std::vector<foeGfxDelayedDestructorFn>>{
                initialDelay, std::vector<foeGfxDelayedDestructorFn>{}},
    };
    pNewDelayedDestructor->currentList = pNewDelayedDestructor->fnLists.begin();

    *pDelayedDestructor = delayed_destructor_to_handle(pNewDelayedDestructor);

    return std::error_code{};
}

void foeGfxDestroyDelayedDestructor(foeGfxDelayedDestructor delayedDestructor) {
    auto *pDelayedDestructor = delayed_destructor_from_handle(delayedDestructor);

    // Go through all of the remaining call lists rapidly, then destroy
    pDelayedDestructor->sync.lock();

    auto callList = pDelayedDestructor->currentList;
    do {
        ++callList;
        if (callList == pDelayedDestructor->fnLists.end()) {
            callList = pDelayedDestructor->fnLists.begin();
        }

        for (auto &call : *callList) {
            call(pDelayedDestructor->session);
        }
    } while (callList != pDelayedDestructor->currentList);

    delete pDelayedDestructor;
}

void foeGfxRunDelayedDestructor(foeGfxDelayedDestructor delayedDestructor) {
    auto *pDelayedDestructor = delayed_destructor_from_handle(delayedDestructor);

    pDelayedDestructor->sync.lock();
    // Increment the current call list
    ++pDelayedDestructor->currentList;
    if (pDelayedDestructor->currentList == pDelayedDestructor->fnLists.end()) {
        pDelayedDestructor->currentList = pDelayedDestructor->fnLists.begin();
    }
    // Grab the calls to run for this run
    auto callsToRun = std::move(*pDelayedDestructor->currentList);
    pDelayedDestructor->sync.unlock();

    for (auto &call : callsToRun) {
        call(pDelayedDestructor->session);
    }
}

void foeGfxAddDelayedDestructionCall(foeGfxDelayedDestructor delayedDestructor,
                                     foeGfxDelayedDestructorFn fn) {
    auto *pDelayedDestructor = delayed_destructor_from_handle(delayedDestructor);

    foeGfxAddDelayedDestructionCall(delayedDestructor, fn, pDelayedDestructor->currentDelay);
}

void foeGfxAddDelayedDestructionCall(foeGfxDelayedDestructor delayedDestructor,
                                     foeGfxDelayedDestructorFn fn,
                                     uint32_t numDelayed) {
    auto *pDelayedDestructor = delayed_destructor_from_handle(delayedDestructor);

    // If supposed to be 'immediate', set it to be destroyed next run
    if (numDelayed == 0) {
        numDelayed = 1;
    }

    pDelayedDestructor->sync.lock();

    if (numDelayed > pDelayedDestructor->fnLists.size()) {
        // Need to enlarge the number of function lists to accomodate a delayed call for further
        // into the future than currently supported
        std::vector<std::vector<foeGfxDelayedDestructorFn>> newList;
        newList.reserve(numDelayed);

        auto fnList = pDelayedDestructor->currentList;
        do {
            ++fnList;
            if (fnList == pDelayedDestructor->fnLists.end()) {
                fnList = pDelayedDestructor->fnLists.begin();
            }

            newList.emplace_back(std::move(*fnList));
        } while (fnList != pDelayedDestructor->currentList);

        newList.resize(numDelayed);

        // Set the data
        pDelayedDestructor->fnLists = std::move(newList);
        pDelayedDestructor->currentList = pDelayedDestructor->fnLists.end() - 1;
    }

    // Add the fn at the desired delay
    auto fnList = pDelayedDestructor->currentList;
    if (numDelayed != pDelayedDestructor->fnLists.size()) {
        for (; numDelayed > 0; --numDelayed, ++fnList) {
            if (fnList == pDelayedDestructor->fnLists.end()) {
                fnList = pDelayedDestructor->fnLists.begin();
            }
        }
    }
    fnList->emplace_back(fn);

    pDelayedDestructor->sync.unlock();
}
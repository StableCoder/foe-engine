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

#include "delayed_destructor.hpp"

#include <vk_error_code.hpp>

#include "session.hpp"

auto foeGfxCreateDelayedDestructor(foeGfxSession session,
                                   foeGfxDelayedDestructor *pDelayedDestructor) -> std::error_code {
    auto *pSession = session_from_handle(session);

    auto *pNewDelayedDestructor = new foeGfxVkDelayedDestructor{
        .pGfxSession = pSession,
        .fnLists =
            std::vector<std::vector<foeGfxVkDelayedDestructorFn>>{
                1, std::vector<foeGfxVkDelayedDestructorFn>{}},
    };
    pNewDelayedDestructor->currentList = pNewDelayedDestructor->fnLists.begin();

    *pDelayedDestructor = delayed_destructor_to_handle(pNewDelayedDestructor);
    return VK_SUCCESS;
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
            call(pDelayedDestructor->pGfxSession->device,
                 pDelayedDestructor->pGfxSession->allocator);
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
        call(pDelayedDestructor->pGfxSession->device, pDelayedDestructor->pGfxSession->allocator);
    }
}

void foeGfxVkAddDelayedDestructionCall(foeGfxVkDelayedDestructor *pVkDelayedDestructor,
                                       foeGfxVkDelayedDestructorFn fn,
                                       uint32_t numDelayed) {
    // if supposed to be 'immediate', set it to be destroyed next run
    if (numDelayed == 0) {
        numDelayed = 1;
    }

    pVkDelayedDestructor->sync.lock();

    if (numDelayed > pVkDelayedDestructor->fnLists.size()) {
        // Need to enlarge the number of function lists to accomodate a delayed call for further
        // into the future than currently supported
        std::vector<std::vector<foeGfxVkDelayedDestructorFn>> newList;
        newList.reserve(numDelayed);

        auto fnList = pVkDelayedDestructor->currentList;
        do {
            ++fnList;
            if (fnList == pVkDelayedDestructor->fnLists.end()) {
                fnList = pVkDelayedDestructor->fnLists.begin();
            }

            newList.emplace_back(std::move(*fnList));
        } while (fnList != pVkDelayedDestructor->currentList);

        newList.resize(numDelayed);

        // Set the data
        pVkDelayedDestructor->fnLists = std::move(newList);
        pVkDelayedDestructor->currentList = pVkDelayedDestructor->fnLists.end() - 1;
    }

    // Add the fn at the desired delay
    auto fnList = pVkDelayedDestructor->currentList;
    if (numDelayed != pVkDelayedDestructor->fnLists.size()) {
        for (; numDelayed > 0; --numDelayed, ++fnList) {
            if (fnList == pVkDelayedDestructor->fnLists.end()) {
                fnList = pVkDelayedDestructor->fnLists.begin();
            }
        }
    }
    fnList->emplace_back(fn);

    pVkDelayedDestructor->sync.unlock();
}
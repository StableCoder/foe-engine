// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/delayed_caller.h>

#include "log.hpp"

#include <atomic>
#include <mutex>
#include <vector>

namespace {

struct DelayedCall {
    PFN_foeGfxDelayedCall destroyCall;
    void *pContext;
};

struct foeGfxDelayedCallerImpl {
    foeGfxSession const session;

    std::atomic_uint currentDelay;
    std::mutex sync;
    std::vector<std::vector<DelayedCall>> callLists;
    std::vector<std::vector<DelayedCall>>::iterator currentList;
};

FOE_DEFINE_HANDLE_CASTS(delayed_destructor, foeGfxDelayedCallerImpl, foeGfxDelayedCaller)

} // namespace

extern "C" foeResult foeGfxCreateDelayedCaller(foeGfxSession session,
                                               uint32_t initialDelay,
                                               foeGfxDelayedCaller *pDelayedCaller) {
    auto *pNewDelayedDestructor = new foeGfxDelayedCallerImpl{
        .session = session,
        .currentDelay = initialDelay,
        .callLists =
            std::vector<std::vector<DelayedCall>>{initialDelay, std::vector<DelayedCall>{}},
    };
    pNewDelayedDestructor->currentList = pNewDelayedDestructor->callLists.begin();

    *pDelayedCaller = delayed_destructor_to_handle(pNewDelayedDestructor);

    FOE_LOG(foeGraphics, Verbose, "[{}] foeGfxDelayedCaller - Created using foeGfxSession {}",
            static_cast<void *>(pNewDelayedDestructor), (void *)session);

    return foeResult{.value = FOE_SUCCESS, .toString = NULL};
}

extern "C" void foeGfxDestroyDelayedCaller(foeGfxDelayedCaller delayedCaller) {
    auto *pDelayedCaller = delayed_destructor_from_handle(delayedCaller);

    FOE_LOG(foeGraphics, Verbose, "[{}] foeGfxDelayedCaller - Destroying",
            static_cast<void *>(delayedCaller));

    // Go through all of the remaining call lists rapidly, then destroy
    pDelayedCaller->sync.lock();
    auto callList = pDelayedCaller->currentList;
    do {
        ++callList;
        if (callList == pDelayedCaller->callLists.end()) {
            callList = pDelayedCaller->callLists.begin();
        }

        for (auto const &call : *callList) {
            call.destroyCall(call.pContext, pDelayedCaller->session);
        }
    } while (callList != pDelayedCaller->currentList);
    pDelayedCaller->sync.unlock();

    delete pDelayedCaller;

    FOE_LOG(foeGraphics, Verbose, "[{}] foeGfxDelayedCaller - Destroyed",
            static_cast<void *>(delayedCaller));
}

extern "C" void foeGfxRunDelayedCalls(foeGfxDelayedCaller delayedCaller) {
    auto *pDelayedCaller = delayed_destructor_from_handle(delayedCaller);

    pDelayedCaller->sync.lock();
    // Increment the current call list
    ++pDelayedCaller->currentList;
    if (pDelayedCaller->currentList == pDelayedCaller->callLists.end()) {
        pDelayedCaller->currentList = pDelayedCaller->callLists.begin();
    }
    // Grab the calls to run for this run
    auto callsToRun = std::move(*pDelayedCaller->currentList);
    pDelayedCaller->sync.unlock();

    for (auto const &call : callsToRun) {
        call.destroyCall(call.pContext, pDelayedCaller->session);
    }
}

extern "C" void foeGfxAddDefaultDelayedCall(foeGfxDelayedCaller delayedCaller,
                                            PFN_foeGfxDelayedCall destroyCall,
                                            void *pContext) {
    auto *pDelayedCaller = delayed_destructor_from_handle(delayedCaller);

    foeGfxAddDelayedCall(delayedCaller, destroyCall, pContext, pDelayedCaller->currentDelay);
}

extern "C" void foeGfxAddDelayedCall(foeGfxDelayedCaller delayedCaller,
                                     PFN_foeGfxDelayedCall destroyCall,
                                     void *pContext,
                                     uint32_t numDelayed) {
    auto *pDelayedCaller = delayed_destructor_from_handle(delayedCaller);

    // If supposed to be 'immediate', set it to be destroyed next run
    if (numDelayed == 0) {
        numDelayed = 1;
    }

    pDelayedCaller->sync.lock();

    if (numDelayed > pDelayedCaller->callLists.size()) {
        // Need to enlarge the number of function lists to accomodate a delayed call for further
        // into the future than currently supported
        FOE_LOG(foeGraphics, Verbose, "[{}] foeGfxDelayedCaller - Expanding delay from {} to {}",
                static_cast<void *>(delayedCaller), pDelayedCaller->callLists.size(), numDelayed);

        pDelayedCaller->currentList = pDelayedCaller->callLists.end() - 1;

        std::vector<std::vector<DelayedCall>> newList;
        newList.reserve(numDelayed);

        auto fnList = pDelayedCaller->currentList;
        do {
            ++fnList;
            if (fnList == pDelayedCaller->callLists.end()) {
                fnList = pDelayedCaller->callLists.begin();
            }

            newList.emplace_back(std::move(*fnList));
        } while (fnList != pDelayedCaller->currentList);

        newList.resize(numDelayed);

        // Set the data
        pDelayedCaller->callLists = std::move(newList);
    }

    // Add the fn at the desired delay
    auto fnList = pDelayedCaller->currentList;
    if (numDelayed != pDelayedCaller->callLists.size()) {
        for (; numDelayed > 0; --numDelayed, ++fnList) {
            if (fnList == pDelayedCaller->callLists.end()) {
                fnList = pDelayedCaller->callLists.begin();
            }
        }
        if (fnList == pDelayedCaller->callLists.end()) {
            fnList = pDelayedCaller->callLists.begin();
        }
    }
    fnList->emplace_back(DelayedCall{
        .destroyCall = destroyCall,
        .pContext = pContext,
    });

    pDelayedCaller->sync.unlock();
}
// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "network_initialization.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "result.h"

#include <atomic>
#include <mutex>

namespace {

std::mutex gNetworkInitSync;
std::atomic<int> gNetworkInitCount = {0};

} // namespace

foeResultSet initializeNetworkStack() {
    std::unique_lock lock{gNetworkInitSync};

    int initCount = ++gNetworkInitCount;

    if (initCount == 1) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return to_foeResult(FOE_NETWORK_ERROR_WSA_STARTUP_FAILED);
        }

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
            return to_foeResult(FOE_NETWORK_ERROR_WSA_INCORRECT_VERSION);
        }
    }

    return to_foeResult(FOE_NETWORK_SUCCESS);
}

void deinitializeNetworkStack() {
    std::unique_lock lock{gNetworkInitSync};

    int initCount = --gNetworkInitCount;

    if (initCount == 0) {
        WSACleanup();
    }
}

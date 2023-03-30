// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/network/address.h>

#include "address.h"
#include "network_initialization.h"
#include "result.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(sizeof(foeNetworkAddress) == sizeof(struct in6_addr),
               "foeNetworkAddress must be same size for IPv6 address");
_Static_assert(FOE_NETWORK_ADDRESS_STRLEN == INET6_ADDRSTRLEN,
               "FOE_NETWORK_ADDRESS_STRLEN must be large enough to fit full IPv6 in stirng form");

bool foeNetworkAddressIsIPv4(foeNetworkAddress address) {
    // First 9 bytes are 0x00
    for (int i = 0; i < 10; ++i) {
        if (address.ip_block[i] != 0x00)
            return false;
    }

    // Bytes 10/11 are 0xFF
    return address.ip_block[10] == 0xFF && address.ip_block[11] == 0xFF;
}

bool foeNetworkAddressIsLoopback(foeNetworkAddress address) {
    if (foeNetworkAddressIsIPv4(address)) {
        // Check for 127.0.0.1
        return address.ip_block[12] == 127 && address.ip_block[13] == 0 &&
               address.ip_block[14] == 0 && address.ip_block[15] == 1;
    }

    // First 14 bytes are 0x00
    for (int i = 0; i < 15; ++i) {
        if (address.ip_block[i] != 0x00)
            return false;
    }
    // Last byte is 1
    return address.ip_block[15] == 0x01;
}

foeResultSet foeNetworkEnumerateAddresses(char const *pAddressStr,
                                          uint32_t *pAddressCount,
                                          foeNetworkAddress *pAddresses) {
    foeResultSet result = initializeNetworkStack();
    if (result.value != FOE_SUCCESS)
        return result;

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
    };

    struct addrinfo *addrInfos = NULL;
    int errCode = getaddrinfo(pAddressStr, NULL, &hints, &addrInfos);
    if (errCode != 0) {
        result = to_foeResult(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED);
        goto ENUMERATE_ADDRESSES_FAILED;
    }

    struct addrinfo *address = addrInfos;
    uint32_t addrCount = 0;
    if (pAddresses == NULL) {
        // Only counting the results
        for (; address != NULL; address = address->ai_next, ++addrCount) {
            switch (address->ai_family) {
            case AF_INET:
            case AF_INET6:
                break;
            default:
                // Uncount this address we don't support
                --addrCount;
                break;
            }
        }
    } else {
        // Output upto given count of addresses
        for (; address != NULL && addrCount < *pAddressCount;
             address = address->ai_next, ++addrCount) {
            switch (address->ai_family) {
            case AF_INET:
                *pAddresses = setAddressIPv4((struct sockaddr_in *)address->ai_addr);
                break;
            case AF_INET6:
                *pAddresses = setAddressIPv6((struct sockaddr_in6 *)address->ai_addr);
                break;
            default:
                // Uncount this address we don't support
                continue;
            }

            ++pAddresses;
        }
    }

    *pAddressCount = addrCount;

    result = (address == NULL) ? to_foeResult(FOE_NETWORK_SUCCESS)
                               : to_foeResult(FOE_NETWORK_INCOMPLETE);
    freeaddrinfo(addrInfos);

ENUMERATE_ADDRESSES_FAILED:
    deinitializeNetworkStack();
    return result;
}

#ifndef _WIN32
foeResultSet foeNetworkEnumerateInterfaces(uint32_t *pInterfaceCount,
                                           foeNetworkInterface *pInterfaces) {
    foeResultSet result = initializeNetworkStack();
    if (result.value != FOE_SUCCESS)
        return result;

    struct ifaddrs *ifAddrs = NULL;
    int errCode = getifaddrs(&ifAddrs);
    if (errCode != 0) {
        result = to_foeResult(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED);
        goto ENUMERATE_INTERFACES_FAILED;
    }

    struct ifaddrs *interface = ifAddrs;
    uint32_t interfaceCount = 0;
    bool strIncomplete = false;
    if (pInterfaces == NULL) {
        // Only count the results
        for (; interface != NULL; interface = interface->ifa_next, ++interfaceCount) {
            switch (interface->ifa_addr->sa_family) {
            case AF_INET:
            case AF_INET6:
                break;
            default:
                // Uncount this interface we don't support
                --interfaceCount;
                break;
            }
        }
    } else {
        // Output upto given count of interfaces
        for (; interface != NULL && interfaceCount < *pInterfaceCount;
             interface = interface->ifa_next, ++interfaceCount) {
            switch (interface->ifa_addr->sa_family) {
            case AF_INET:
                pInterfaces->address = setAddressIPv4((struct sockaddr_in *)interface->ifa_addr);
                break;
            case AF_INET6:
                pInterfaces->address = setAddressIPv6((struct sockaddr_in6 *)interface->ifa_addr);
                break;
            default:
                // Uncount this interface we don't support
                --interfaceCount;
                continue;
            }

            // If the name >= than the capacity provided by the structure, we want to return a
            // particulare error code
            if (strlen(interface->ifa_name) >= sizeof(pInterfaces->description))
                strIncomplete = true;

            strncpy(&pInterfaces->description[0], interface->ifa_name,
                    sizeof(pInterfaces->description) - 1);
            // Make sure the string is always null-terminated
            pInterfaces->description[sizeof(pInterfaces->description) - 1] = '\0';

            ++pInterfaces;
        }
    }

    *pInterfaceCount = interfaceCount;
    result = (interface == NULL) ? to_foeResult(FOE_NETWORK_SUCCESS)
                                 : to_foeResult(FOE_NETWORK_INCOMPLETE);
    if (result.value == FOE_NETWORK_SUCCESS && strIncomplete)
        result = to_foeResult(FOE_NETWORK_STRING_INCOMPLETE);
    freeifaddrs(ifAddrs);

ENUMERATE_INTERFACES_FAILED:
    deinitializeNetworkStack();
    return result;
}

#endif

void foeNetworkAddressToString(foeNetworkAddress address,
                               char addrStr[FOE_NETWORK_ADDRESS_STRLEN]) {
    int ipFamily = AF_INET6;
    void const *pAddr = &address;

    if (foeNetworkAddressIsIPv4(address)) {
        ipFamily = AF_INET;
        pAddr = &address.ip_block[12];
    }

    inet_ntop(ipFamily, pAddr, addrStr, FOE_NETWORK_ADDRESS_STRLEN);
}

foeResultSet foeNetworkLocalHostname(char *pHostname, int len) {
    foeResultSet result = initializeNetworkStack();
    if (result.value != FOE_SUCCESS)
        return result;

    int errCode = gethostname(pHostname, len);
    if (errCode != 0)
        result = to_foeResult(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED);

    deinitializeNetworkStack();
    return result;
}
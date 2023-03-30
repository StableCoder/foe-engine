// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/network/address.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define __USE_XOPEN2K // Enable `struct addrinfo` and associated
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string.h>

static inline struct sockaddr_in6 getAddressIPv6(foeNetworkAddress address) {
    struct sockaddr_in6 outAddr;
    memset(&outAddr, 0x00, sizeof(outAddr));

    outAddr.sin6_family = AF_INET6;
    memcpy(&outAddr.sin6_addr, &address.ip_block[0], 16);

    return outAddr;
}

static inline foeNetworkAddress setAddressIPv4(struct sockaddr_in const *pInAddr) {
    foeNetworkAddress outAddr;
    memset(&outAddr, 0x00, sizeof(outAddr));

    // First 9 bytes are 0x00
    // Bytes 10/11 are 0xFF
    outAddr.ip_block[10] = 0xFF;
    outAddr.ip_block[11] = 0xFF;
    // Last 4 bytes are the IPv4 address bytes
    memcpy(&outAddr.ip_block[12], &pInAddr->sin_addr, 4);

    return outAddr;
}

static inline foeNetworkAddress setAddressIPv6(struct sockaddr_in6 const *pInAddr) {
    foeNetworkAddress outAddr;

    memcpy(&outAddr.ip_block[0], &pInAddr->sin6_addr, 16);

    return outAddr;
}
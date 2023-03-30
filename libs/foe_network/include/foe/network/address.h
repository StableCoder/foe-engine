// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_NETWORK_ADDRESS_H
#define FOE_NETWORK_ADDRESS_H

#include <foe/network/export.h>
#include <foe/result.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define FOE_NETWORK_ADDRESS_STRLEN 65U
#else
#define FOE_NETWORK_ADDRESS_STRLEN 46U
#endif

typedef struct foeNetworkAddress {
    uint8_t ip_block[16];
} foeNetworkAddress;

typedef struct foeNetworkInterface {
    foeNetworkAddress address;
    char description[16];
} foeNetworkInterface;

FOE_NETWORK_EXPORT
bool foeNetworkAddressIsIPv4(foeNetworkAddress address);

FOE_NETWORK_EXPORT
bool foeNetworkAddressIsLoopback(foeNetworkAddress address);

FOE_NETWORK_EXPORT
foeResultSet foeNetworkEnumerateAddresses(char const *pAddressStr,
                                          uint32_t *pAddressCount,
                                          foeNetworkAddress *pAddresses);

#ifndef _WIN32 // Only have a unix implementation currently
FOE_NETWORK_EXPORT
foeResultSet foeNetworkEnumerateInterfaces(uint32_t *pInterfaceCount,
                                           foeNetworkInterface *pInterfaces);
#endif

FOE_NETWORK_EXPORT
void foeNetworkAddressToString(foeNetworkAddress address, char addrStr[FOE_NETWORK_ADDRESS_STRLEN]);

FOE_NETWORK_EXPORT
foeResultSet foeNetworkLocalHostname(char *pHostname, int len);

#ifdef __cplusplus
}
#endif

#endif // FOE_NETWORK_ADDRESS_H
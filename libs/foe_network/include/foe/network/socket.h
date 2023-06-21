// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_NETWORK_SOCKET_H
#define FOE_NETWORK_SOCKET_H

#include <foe/handle.h>
#include <foe/network/address.h>
#include <foe/network/export.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeNetworkSocket);

FOE_NETWORK_EXPORT
foeResultSet foeCreateNetworkSocket(foeNetworkAddress address,
                                    uint16_t port,
                                    foeNetworkSocket *pSocket);

FOE_NETWORK_EXPORT
void foeDestroyNetworkSocket(foeNetworkSocket socket);

FOE_NETWORK_EXPORT
#ifndef _WIN32
int foeNetworkSocketGetHandle(foeNetworkSocket socket);
#else
SOCKET foeNetworkSocketGetHandle(foeNetworkSocket socket);
#endif

FOE_NETWORK_EXPORT
foeResultSet foeNetworkGetSocketAddress(foeNetworkSocket socket,
                                        foeNetworkAddress *pAddress,
                                        uint16_t *pPort);

FOE_NETWORK_EXPORT
foeResultSet foeNetworkSocketSendData(
    foeNetworkSocket socket, int size, void const *pData, foeNetworkAddress address, uint16_t port);

FOE_NETWORK_EXPORT
foeResultSet foeNetworkSocketRecvData(foeNetworkSocket socket,
                                      int *pBufferSize,
                                      void *pBuffer,
                                      foeNetworkAddress *pAddress,
                                      uint16_t *pPort);

#ifdef __cplusplus
}
#endif

#endif // FOE_NETWORK_SOCKET_H
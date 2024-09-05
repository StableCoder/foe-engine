// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/network/socket.h>

#include "address.h"
#include "network_initialization.h"
#include "result.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
typedef SOCKET foePlatformSocket;
#else
#include <sys/ioctl.h>

typedef int foePlatformSocket;
#endif

_Static_assert(sizeof(foeNetworkSocket) >= sizeof(int),
               "foeNetworkSocket must be large enough for the socket file descriptor");

static foeNetworkSocket socket_to_handle(foePlatformSocket socket) {
    return (foeNetworkSocket)socket;
}

static int socket_from_handle(foeNetworkSocket handle) { return (foePlatformSocket)handle; }

foeResultSet foeCreateNetworkSocket(foeNetworkAddress address,
                                    uint16_t port,
                                    foeNetworkSocket *pSocket) {
    if (port == 0)
        return to_foeResult(FOE_NETWORK_ERROR_INVALID_PORT);

    foeResultSet result = initializeNetworkStack();
    if (result.value != FOE_SUCCESS)
        return result;

    // Socket
    foePlatformSocket newSocket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (newSocket == -1) {
        deinitializeNetworkStack();
        return to_foeResult(FOE_NETWORK_ERROR_SOCKET_CREATION_ERROR);
    }

    // Set socket to non-blocking
    unsigned long opt = 1;
#ifdef _WIN32
    if (ioctlsocket(newSocket, FIONBIO, &opt) != 0)
#else
    if (ioctl(newSocket, FIONBIO, &opt) != 0)
#endif
    {
        foeDestroyNetworkSocket(socket_to_handle(newSocket));
        return to_foeResult(FOE_NETWORK_ERROR_SOCKET_FAILED_TO_SET_NONBLOCKING);
    }

    // Set Socket to be dual stack (IPv4 and IPv6)
    // Mostly a Windows issue, Unix sockets are dual-stack by default
    int val = 0;
#ifdef __MINGW32__
    int errCode = setsockopt(newSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char const *)&val, sizeof(val));
#else
    int errCode = setsockopt(newSocket, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));
#endif
    if (errCode != 0) {
        foeDestroyNetworkSocket(socket_to_handle(newSocket));
        return to_foeResult(FOE_NETWORK_ERROR_SOCKET_FAILED_TO_SET_DUAL_STACK);
    }

    // Bind socket to interface/address and port
    struct sockaddr_in6 bindAddress = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(port),
    };
    memcpy(&bindAddress.sin6_addr, &address, sizeof(struct in6_addr));

    errCode = bind(newSocket, (struct sockaddr const *)&bindAddress, sizeof(struct sockaddr_in6));
    if (errCode != 0) {
        foeDestroyNetworkSocket(socket_to_handle(newSocket));
        return to_foeResult(FOE_NETWORK_ERROR_SOCKET_BIND_ERROR);
    }

    *pSocket = socket_to_handle(newSocket);
    return result;
}

void foeDestroyNetworkSocket(foeNetworkSocket socket) {
#ifdef _WIN32
    closesocket(socket_from_handle(socket));
#else
    close(socket_from_handle(socket));
#endif
    deinitializeNetworkStack();
}

#ifndef _WIN32
int foeNetworkSocketGetHandle(foeNetworkSocket socket)
#else
SOCKET foeNetworkSocketGetHandle(foeNetworkSocket socket)
#endif
{
    return socket_from_handle(socket);
}

foeResultSet foeNetworkGetSocketAddress(foeNetworkSocket socket,
                                        foeNetworkAddress *pAddress,
                                        uint16_t *pPort) {
    // Don't try network stack de/init, this should only really be happening on an already-existing
    // socket

    struct sockaddr_storage address;
    socklen_t addressLen = sizeof(struct sockaddr_storage);
    if (getsockname(socket_from_handle(socket), (struct sockaddr *)&address, &addressLen) != 0) {
        return to_foeResult(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED);
    }

    if (address.ss_family != AF_INET6)
        return to_foeResult(FOE_NETWORK_ERROR_NOT_IPV6);

    struct sockaddr_in6 const *pAddr6 = (struct sockaddr_in6 const *)&address;
    memcpy(pAddress, &pAddr6->sin6_addr, sizeof(foeNetworkAddress));

    *pPort = ntohs(pAddr6->sin6_port);

    return to_foeResult(FOE_NETWORK_SUCCESS);
}

foeResultSet foeNetworkSocketSendData(foeNetworkSocket socket,
                                      int size,
                                      void const *pData,
                                      foeNetworkAddress address,
                                      uint16_t port) {
    struct sockaddr_in6 toAddr = getAddressIPv6(address);
    toAddr.sin6_port = htons(port);

    int sent = sendto(socket_from_handle(socket), pData, size, 0, (struct sockaddr *)&toAddr,
                      sizeof(toAddr));

    if (sent == size)
        return to_foeResult(FOE_NETWORK_SUCCESS);
    if (sent == -1)
        return to_foeResult(FOE_NETWORK_ERROR_SEND_FAILURE);

    // Otherwise a partial send?
    return to_foeResult(FOE_NETWORK_ERROR_PARTIAL_SEND);
}

foeResultSet foeNetworkSocketRecvData(foeNetworkSocket socket,
                                      int *pBufferSize,
                                      void *pBuffer,
                                      foeNetworkAddress *pAddress,
                                      uint16_t *pPort) {
    struct sockaddr_storage fromAddr;
    socklen_t fromAddrSize = sizeof(fromAddr);

    int recvSize = recvfrom(socket_from_handle(socket), pBuffer, *pBufferSize, 0,
                            (struct sockaddr *)&fromAddr, &fromAddrSize);
    if (recvSize >= 0)
        *pBufferSize = recvSize;
    else
        // Most of the time it's -1, usually meaning it would have blocked.
        // @TODO Implement better error checking
        return to_foeResult(FOE_NETWORK_NO_DATA_READ);

    switch (fromAddr.ss_family) {
    case AF_INET:
        *pAddress = setAddressIPv4((struct sockaddr_in *)&fromAddr);
        *pPort = ntohs(((struct sockaddr_in *)&fromAddr)->sin_port);
        break;
    case AF_INET6:
        *pAddress = setAddressIPv6((struct sockaddr_in6 *)&fromAddr);
        *pPort = ntohs(((struct sockaddr_in6 *)&fromAddr)->sin6_port);
        break;
    default:
        // Unsupported address family type
        return to_foeResult(FOE_NETWORK_ERROR_UNSUPPORTED_FAMILY_TYPE);
    }

    return to_foeResult(FOE_NETWORK_SUCCESS);
}
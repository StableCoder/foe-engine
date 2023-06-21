// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/network/result.h>
#include <foe/network/socket.h>

#include <chrono>
#include <cstring>
#include <thread>

namespace {

constexpr foeNetworkAddress cBlankAddress = {};
constexpr uint16_t cTestPort = 4200;
constexpr uint16_t cSendPort = 4201;

constexpr std::string_view cSendData = "Hello, World!";

} // namespace

TEST_CASE("foeNetwork - Creating a socket with port of 0 fails") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;

    foeResultSet result = foeCreateNetworkSocket(cBlankAddress, 0, &socket);
    REQUIRE(result.value == FOE_NETWORK_ERROR_INVALID_PORT);
    REQUIRE(socket == FOE_NULL_HANDLE);
}

TEST_CASE("foeNetwork - Creating and destroying a socket on ANY ADDRESS") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;

    foeResultSet result = foeCreateNetworkSocket(cBlankAddress, cTestPort, &socket);
    REQUIRE(socket != FOE_NULL_HANDLE);

    // Check the address of the socket
    foeNetworkAddress boundAddr = {0xFF};
    uint16_t boundPort = 0;

    result = foeNetworkGetSocketAddress(socket, &boundAddr, &boundPort);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(memcmp(&boundAddr, &cBlankAddress, sizeof(foeNetworkAddress)) == 0);
    CHECK(boundPort == cTestPort);

    foeDestroyNetworkSocket(socket);
}

TEST_CASE("foeNetwork - Creating and destroying a socket on IPv6 loopback") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;
    foeNetworkAddress addr = {};
    uint32_t addrCount = 1;

    foeResultSet result = foeNetworkEnumerateAddresses("::1", &addrCount, &addr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 1);

    result = foeCreateNetworkSocket(addr, cTestPort, &socket);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(socket != FOE_NULL_HANDLE);

    // Check the address of the socket
    foeNetworkAddress boundAddr = {};
    uint16_t boundPort = 0;

    result = foeNetworkGetSocketAddress(socket, &boundAddr, &boundPort);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(memcmp(&boundAddr, &addr, sizeof(foeNetworkAddress)) == 0);
    CHECK(boundPort == cTestPort);

    SECTION("Send data to socket: ANY_ADDR -> IPv6") {
        foeNetworkSocket sendSocket = FOE_NULL_HANDLE;

        result = foeCreateNetworkSocket({}, cSendPort, &sendSocket);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeNetworkSocketSendData(sendSocket, cSendData.size(), cSendData.data(), addr,
                                          cTestPort);
        REQUIRE(result.value == FOE_SUCCESS);

        uint8_t recvBuffer[cSendData.size() * 2];
        int recvSize = sizeof(recvBuffer);
        foeNetworkAddress fromAddr = {};
        uint16_t fromPort = 0;

        // Loop waiting for the message
        auto haltTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(15);
        do {
            result = foeNetworkSocketRecvData(socket, &recvSize, recvBuffer, &fromAddr, &fromPort);
        } while (result.value == FOE_NETWORK_NO_DATA_READ ||
                 haltTime < std::chrono::steady_clock::now());

        REQUIRE(result.value == FOE_SUCCESS);

        CHECK_FALSE(foeNetworkAddressIsIPv4(fromAddr));
        CHECK(foeNetworkAddressIsLoopback(fromAddr));
        CHECK(fromPort == cSendPort);

        REQUIRE(recvSize == cSendData.size());
        CHECK(memcmp(recvBuffer, cSendData.data(), cSendData.size()) == 0);

        foeDestroyNetworkSocket(sendSocket);
    }

    foeDestroyNetworkSocket(socket);
}

TEST_CASE("foeNetwork - Creating and destroying a socket on IPv4 loopback") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;
    foeNetworkAddress addr = {};
    uint32_t addrCount = 1;

    foeResultSet result = foeNetworkEnumerateAddresses("::ffff:127.0.0.1", &addrCount, &addr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 1);

    result = foeCreateNetworkSocket(addr, cTestPort, &socket);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(socket != FOE_NULL_HANDLE);

    // Check the address of the socket
    foeNetworkAddress boundAddr = {};
    uint16_t boundPort = 0;

    result = foeNetworkGetSocketAddress(socket, &boundAddr, &boundPort);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(memcmp(&boundAddr, &addr, sizeof(foeNetworkAddress)) == 0);
    CHECK(boundPort == cTestPort);

    SECTION("Send data to socket: ANY_ADDR -> IPv4") {
        foeNetworkSocket sendSocket = FOE_NULL_HANDLE;

        result = foeCreateNetworkSocket({}, cSendPort, &sendSocket);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeNetworkSocketSendData(sendSocket, cSendData.size(), cSendData.data(), addr,
                                          cTestPort);
        REQUIRE(result.value == FOE_SUCCESS);

        uint8_t recvBuffer[cSendData.size() * 2];
        int recvSize = sizeof(recvBuffer);
        foeNetworkAddress fromAddr = {};
        uint16_t fromPort = 0;

        // Loop waiting for the message
        auto haltTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(15);
        do {
            result = foeNetworkSocketRecvData(socket, &recvSize, recvBuffer, &fromAddr, &fromPort);
        } while (result.value == FOE_NETWORK_NO_DATA_READ ||
                 haltTime < std::chrono::steady_clock::now());

        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(foeNetworkAddressIsIPv4(fromAddr));
        CHECK(foeNetworkAddressIsLoopback(fromAddr));
        CHECK(fromPort == cSendPort);

        REQUIRE(recvSize == cSendData.size());
        CHECK(memcmp(recvBuffer, cSendData.data(), cSendData.size()) == 0);

        foeDestroyNetworkSocket(sendSocket);
    }

    foeDestroyNetworkSocket(socket);
}

TEST_CASE("foeNetwork - Sending from bad socket fails") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;
    foeNetworkAddress addr;
    foeResultSet result;

    result = foeNetworkSocketSendData(socket, cSendData.size(), cSendData.data(), addr, cTestPort);
    REQUIRE(result.value == FOE_NETWORK_ERROR_SEND_FAILURE);
}

TEST_CASE("foeNetwork - Recieving from a bad socket fails") {
    foeNetworkSocket socket = FOE_NULL_HANDLE;
    uint8_t recvBuffer[cSendData.size() * 2];
    int recvSize = sizeof(recvBuffer);
    foeNetworkAddress fromAddr = {};
    uint16_t fromPort = 0;
    foeResultSet result;

    result = foeNetworkSocketRecvData(socket, &recvSize, recvBuffer, &fromAddr, &fromPort);
    REQUIRE(result.value == FOE_NETWORK_NO_DATA_READ);
}
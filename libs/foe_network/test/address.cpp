// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/network/address.h>
#include <foe/network/result.h>

#include <array>
#include <cstring>
#include <memory>

constexpr foeNetworkAddress cBlankAddress = {}; // All zeroes

TEST_CASE("foeNetwork - Blank/Zeroed Address") {
    foeNetworkAddress address = {};
    memset(&address, 0, sizeof(foeNetworkAddress));

    REQUIRE(memcmp(&address, &cBlankAddress, sizeof(foeNetworkAddress)) == 0);

    CHECK_FALSE(foeNetworkAddressIsIPv4(address));
    CHECK_FALSE(foeNetworkAddressIsLoopback(address));
}

TEST_CASE("foeNetwork - Matching addresses") {
    CHECK(foeNetworkAddressMatch(cBlankAddress, cBlankAddress));

    foeNetworkAddress addr = cBlankAddress;
    addr.ip_block[1] = 0x1;

    CHECK(foeNetworkAddressMatch(addr, addr));

    CHECK_FALSE(foeNetworkAddressMatch(addr, cBlankAddress));
    CHECK_FALSE(foeNetworkAddressMatch(cBlankAddress, addr));

    foeNetworkAddress addr2 = addr;

    CHECK(foeNetworkAddressMatch(addr, addr2));
    CHECK(foeNetworkAddressMatch(addr2, addr));

    addr2.ip_block[4] = 0x2;

    CHECK(foeNetworkAddressMatch(addr2, addr2));

    CHECK_FALSE(foeNetworkAddressMatch(addr, addr2));
    CHECK_FALSE(foeNetworkAddressMatch(addr2, addr));
}

TEST_CASE("foeNetwork - Checking IPv4 Addresses") {
    foeNetworkAddress addr = cBlankAddress;
    REQUIRE(memcmp(&cBlankAddress, &addr, sizeof(foeNetworkAddress)) == 0);

    SECTION("Success Cases") {
        // IP blocks 0-9 are 0x00, and 10/11 are 0xFF
        addr.ip_block[10] = 0xFF;
        addr.ip_block[11] = 0xFF;

        CHECK(foeNetworkAddressIsIPv4(addr));
    }

    SECTION("Any of the first 10 IP blocks are not 0") {
        addr = cBlankAddress;
        for (int i = 0; i < 10; ++i) {
            addr.ip_block[i] = 0x01;

            CHECK_FALSE(foeNetworkAddressIsIPv4(addr));
        }
    }
    SECTION("Either of the IP blocks 10/11 are not 0xFF") {
        addr = cBlankAddress;
        for (int i = 10; i < 12; ++i) {
            addr.ip_block[10] = 0xFF;
            addr.ip_block[11] = 0xFF;

            addr.ip_block[i] = 0x01;

            CHECK_FALSE(foeNetworkAddressIsIPv4(addr));
        }
    }
}

TEST_CASE("foeNetwork - Enumerating addresses") {
    foeResultSet result;
    uint32_t addrCount;
    foeNetworkAddress address = cBlankAddress;

    result = foeNetworkEnumerateAddresses("localhost", &addrCount, nullptr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount > 0);

    // Deliberately attempt to enumerate addresses without having enough to output them all
    addrCount = 1;
    result = foeNetworkEnumerateAddresses("localhost", &addrCount, &address);
    REQUIRE(result.value == FOE_NETWORK_INCOMPLETE);

    // Make sure that at least the one provided is filled out
    CHECK(addrCount == 1);
    CHECK(memcmp(&address, &cBlankAddress, sizeof(foeNetworkAddress)) != 0);
}

#ifndef _WIN32
TEST_CASE("foeNetwork - Enumerating interfaces") {
    foeResultSet result;
    uint32_t interfaceCount;
    std::unique_ptr<foeNetworkInterface[]> interfaces;

    result = foeNetworkEnumerateInterfaces(&interfaceCount, nullptr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(interfaceCount > 1);

    SECTION("Success Case - All interfaces can be retrieved") {
        uint32_t newInterfaceCount = interfaceCount * 2;
        interfaces.reset(new foeNetworkInterface[newInterfaceCount]);

        result = foeNetworkEnumerateInterfaces(&newInterfaceCount, interfaces.get());
        REQUIRE(result.value == FOE_SUCCESS);
        CHECK(newInterfaceCount == interfaceCount);
    }
    SECTION("Failure Case - Nopt enough interfaces provided to output") {
        interfaceCount = 1;
        interfaces.reset(new foeNetworkInterface[1]);

        result = foeNetworkEnumerateInterfaces(&interfaceCount, interfaces.get());
        REQUIRE(result.value == FOE_NETWORK_INCOMPLETE);
        CHECK(interfaceCount == 1);
    }
}
#endif

TEST_CASE("foeNetwork - IPv4 loopback address") {
    foeResultSet result;
    std::array<foeNetworkAddress, 2> addresses = {};
    uint32_t addrCount = 2;
    char ipAddrStr[FOE_NETWORK_ADDRESS_STRLEN];

    SECTION("IPv4 in IPv4 format") {
        // Check that returned count is accurate
        result = foeNetworkEnumerateAddresses("127.0.0.1", &addrCount, nullptr);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(addrCount == 1);
        addrCount = addresses.size();

        // Actually get addresses
        result = foeNetworkEnumerateAddresses("127.0.0.1", &addrCount, addresses.data());
    }
    SECTION("IPv4 in IPv6 format") {
        // Check that returned count is accurate
        result = foeNetworkEnumerateAddresses("::ffff:127.0.0.1", &addrCount, nullptr);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(addrCount == 1);
        addrCount = addresses.size();

        // Actually get addresses
        result = foeNetworkEnumerateAddresses("::ffff:127.0.0.1", &addrCount, addresses.data());
    }
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 1);

    // First item returned is the IPv4 loopback
    CHECK(foeNetworkAddressIsIPv4(addresses[0]));
    CHECK(foeNetworkAddressIsLoopback(addresses[0]));

    foeNetworkAddressToString(addresses[0], ipAddrStr);
    CHECK(std::string(ipAddrStr) == "127.0.0.1");

    // Other entry is still blank
    CHECK(memcmp(&cBlankAddress, &addresses[1], sizeof(foeNetworkAddress)) == 0);
}

TEST_CASE("foeNetwork - IPv6 loopback address") {
    foeResultSet result;
    std::array<foeNetworkAddress, 2> addresses = {};
    uint32_t addrCount = 2;
    char ipAddrStr[FOE_NETWORK_ADDRESS_STRLEN];

    // Check that returned count is accurate
    result = foeNetworkEnumerateAddresses("::1", &addrCount, nullptr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 1);
    addrCount = addresses.size();

    // Actually get addresses
    result = foeNetworkEnumerateAddresses("::1", &addrCount, addresses.data());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 1);

    // First item returned is the IPv6 loopback
    CHECK_FALSE(foeNetworkAddressIsIPv4(addresses[0]));
    CHECK(foeNetworkAddressIsLoopback(addresses[0]));

    foeNetworkAddressToString(addresses[0], ipAddrStr);
    CHECK(std::string(ipAddrStr) == "::1");

    // Other entry is still blank
    CHECK(memcmp(&cBlankAddress, &addresses[1], sizeof(foeNetworkAddress)) == 0);

    SECTION("Any value in IP blocks 0-14 not 0x00 makes it non-loopback") {
        for (int i = 0; i < 15; ++i) {
            foeNetworkAddress testLoopback = addresses[0];
            testLoopback.ip_block[i] = 0x01;

            CHECK_FALSE(foeNetworkAddressIsLoopback(testLoopback));
        }
    }
}

TEST_CASE("foeNetwork - 'localhost' loopback domain address") {
    std::array<foeNetworkAddress, 3> addresses = {};
    uint32_t addrCount = 3;
    char ipAddrStr[FOE_NETWORK_ADDRESS_STRLEN];
    foeResultSet result;

    // Check that returned count is accurate
    result = foeNetworkEnumerateAddresses("localhost", &addrCount, nullptr);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 2);
    addrCount = addresses.size();

    // Actually get addresses
    result = foeNetworkEnumerateAddresses("localhost", &addrCount, addresses.data());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(addrCount == 2);

    // One is IPv4, one is IPv6
    CHECK(((foeNetworkAddressIsIPv4(addresses[0]) && !foeNetworkAddressIsIPv4(addresses[1])) ||
           (!foeNetworkAddressIsIPv4(addresses[0]) && foeNetworkAddressIsIPv4(addresses[1]))));

    // Both are loopback
    CHECK(foeNetworkAddressIsLoopback(addresses[0]));
    CHECK(foeNetworkAddressIsLoopback(addresses[1]));

    // Last entry is still blank
    CHECK(memcmp(&cBlankAddress, &addresses[2], sizeof(foeNetworkAddress)) == 0);
}

TEST_CASE("foeNetwork - Get the hostname") {
    char hostname[256];
    memset(hostname, 0, sizeof(hostname));

    foeResultSet result = foeNetworkLocalHostname(hostname, sizeof(hostname));
    REQUIRE(result.value == FOE_NETWORK_SUCCESS);
    REQUIRE(strlen(hostname) > 0);
}
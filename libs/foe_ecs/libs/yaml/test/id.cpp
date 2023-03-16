// Copyright (C) 2021-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <foe/ecs/group_translator.h>
#include <foe/ecs/result.h>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/exception.hpp>

TEST_CASE("Reading foe ID's with only Index - Success Cases", "[foe][ecs][yaml][id]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 15)"));

    foeId test = FOE_INVALID_ID;
    REQUIRE_NOTHROW(yaml_read_foeResourceID("", root, nullptr, test));
    CHECK(test == foeIdCreate(foeIdPersistentGroup, 15));

    test = FOE_INVALID_ID;
    REQUIRE_NOTHROW(yaml_read_foeEntityID("", root, nullptr, test));
    CHECK(test == foeIdCreate(foeIdPersistentGroup, 15));
}

TEST_CASE("Reading foe ID's with only Index - Failure Cases", "[foe][ecs][yaml][id]") {
    YAML::Node root;

    SECTION("Missing named sub-node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 15)"));
        foeId test = FOE_INVALID_ID;

        CHECK_FALSE(yaml_read_foeResourceID("subId", root, nullptr, test));
        CHECK(test == FOE_INVALID_ID);

        CHECK_FALSE(yaml_read_foeEntityID("subId", root, nullptr, test));
        CHECK(test == FOE_INVALID_ID);
    }

    SECTION("Missing local IndexID") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id_BROKE: 15)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "index_id - Could not find required node to parse foe Index ID"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "index_id - Could not find required node to parse foe Index ID"));
    }

    SECTION("Missing sub-node IndexID") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(subID:
  index_id_BROKE: 15)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("subID", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "subID::index_id - Could not find required node to parse foe Index ID"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("subID", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "subID::index_id - Could not find required node to parse foe Index ID"));
    }

    SECTION("Bad data") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: fifteen)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("index_id - Could not parse Map-type node as 'uint32_t'"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("index_id - Could not parse Map-type node as 'uint32_t'"));
    }
}

TEST_CASE("Reading foeId with for Group & Index - Success Cases", "[foe][ecs][yaml][id]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 15
group_id: 4)"));

    foeId test = FOE_INVALID_ID;
    REQUIRE_NOTHROW(yaml_read_foeResourceID("", root, nullptr, test));
    CHECK(test == foeIdCreate(foeIdValueToGroup(4), 15));

    test = FOE_INVALID_ID;
    REQUIRE_NOTHROW(yaml_read_foeEntityID("", root, nullptr, test));
    CHECK(test == foeIdCreate(foeIdValueToGroup(4), 15));
}

TEST_CASE("Reading foe ID's with Group and Index and GroupTranslator") {
    YAML::Node root;
    foeId test;

    foeEcsGroupTranslator translator{FOE_NULL_HANDLE};
    std::vector<char const *> srcNames = {"10", "15"};
    std::vector<foeIdGroup> srcGroups = {foeIdValueToGroup(10), foeIdValueToGroup(15)};
    std::vector<char const *> dstNames = {"15", "10"};
    std::vector<foeIdGroup> dstGroups = {foeIdValueToGroup(0), foeIdValueToGroup(1)};
    REQUIRE(foeEcsCreateGroupTranslator(srcNames.size(), srcNames.data(), srcGroups.data(),
                                        dstNames.size(), dstNames.data(), dstGroups.data(),
                                        &translator)
                .value == FOE_ECS_SUCCESS);
    REQUIRE(translator != FOE_NULL_HANDLE);

    SECTION("Success Case") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 12
group_id: 15)"));

        test = FOE_INVALID_ID;
        CHECK(yaml_read_foeResourceID("", root, translator, test));
        CHECK(test == foeIdCreate(foeIdValueToGroup(0), 12));

        test = FOE_INVALID_ID;
        CHECK(yaml_read_foeEntityID("", root, translator, test));
        CHECK(test == foeIdCreate(foeIdValueToGroup(0), 12));
    }

    SECTION("Failure Case") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 12
group_id: 14)"));

        CHECK_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, translator, test), foeYamlException,
            Catch::Matchers::Equals("group_id - Was given groupValue of '14' for which no "
                                    "translation exists - FOE_ECS_ERROR_NO_MATCHING_GROUP"));

        CHECK_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, translator, test), foeYamlException,
            Catch::Matchers::Equals("group_id - Was given groupValue of '14' for which no "
                                    "translation exists - FOE_ECS_ERROR_NO_MATCHING_GROUP"));
    }

    foeEcsDestroyGroupTranslator(translator);
}

TEST_CASE("Reading foe ID's with Group & Index - Failure Cases", "[foe][ecs][yaml][id]") {
    YAML::Node root;

    SECTION("Missing index_id node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id_BROKE: 15
group_id: 4)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "index_id - Could not find required node to parse foe Index ID"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals(
                "index_id - Could not find required node to parse foe Index ID"));
    }

    SECTION("Missing group_id node succeeds, but returns using the 'Persistent' group") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 15
group_id_BROKE: 4)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE(yaml_read_foeResourceID("", root, nullptr, test));
        CHECK(test == foeIdCreate(foeIdPersistentGroup, 15));

        test = FOE_INVALID_ID;
        REQUIRE(yaml_read_foeEntityID("", root, nullptr, test));
        CHECK(test == foeIdCreate(foeIdPersistentGroup, 15));
    }

    SECTION("Bad data on index_id") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: fifteen
group_id: 4)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("index_id - Could not parse Map-type node as 'uint32_t'"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("index_id - Could not parse Map-type node as 'uint32_t'"));
    }

    SECTION("Bad data on group_id") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(index_id: 15
group_id: four)"));

        foeId test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeResourceID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("group_id - Could not parse Map-type node as 'uint32_t'"));

        test = FOE_INVALID_ID;
        REQUIRE_THROWS_MATCHES(
            yaml_read_foeEntityID("", root, nullptr, test), foeYamlException,
            Catch::Matchers::Equals("group_id - Could not parse Map-type node as 'uint32_t'"));
    }
}

TEST_CASE("Writing foeId") {
    YAML::Node test;

    SECTION("Persistent Group ID") {
        SECTION("Same node") {
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
        }

        SECTION("Named sub-node") {
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "id_sub", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "id_sub", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
        }

        SECTION("Existing named sub-node") {
            test["id_sub"] = YAML::Node{};
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "id_sub", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "id_sub", foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted == foeIdCreate(foeIdPersistentGroup, foeIdValueToIndex(0x404)));
            }
        }
    }

    SECTION("Custom Group ID") {
        SECTION("Same node") {
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
        }

        SECTION("New named sub-node") {
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "id_sub", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "id_sub", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
        }

        SECTION("Existing named sub-node") {
            test["id_sub"] = YAML::Node{};
            SECTION("foeResourceID") {
                REQUIRE_NOTHROW(yaml_write_foeResourceID(
                    "id_sub", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeResourceID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
            SECTION("foeEntityID") {
                REQUIRE_NOTHROW(yaml_write_foeEntityID(
                    "id_sub", foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)), test));

                foeId reconstituted = FOE_INVALID_ID;
                REQUIRE_NOTHROW(yaml_read_foeEntityID("id_sub", test, nullptr, reconstituted));
                CHECK(reconstituted ==
                      foeIdCreate(foeIdValueToGroup(0x4), foeIdValueToIndex(0x404)));
            }
        }
    }
}

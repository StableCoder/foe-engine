/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <catch.hpp>
#include <foe/ecs/id.hpp>
#include <foe/simulation/group_data.hpp>

#include <filesystem>
#include <string>

static_assert(FOE_SIMULATION_TEST_DATA_DIR != NULL,
              "FOE_SIMULATION_TEST_DATA_DIR cannot be left undefined, check that it is defined in "
              "CMake script!");

class DummyImporter : public foeImporterBase {
  public:
    DummyImporter(std::string_view name, foeIdGroup group, void *resourceReturn = nullptr) :
        mName{name}, mGroup{group}, mResReturn{resourceReturn} {}

    foeIdGroup group() const noexcept final { return mGroup; }
    std::string name() const noexcept final { return mName; }
    void setGroupTranslator(foeIdGroupTranslator &&groupTranslation) final {}

    bool getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) final { return false; }
    bool getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) final { return false; }
    bool getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) final { return false; }
    bool importStateData(foeEditorNameMap *pEntityNameMap,
                         std::vector<foeComponentPoolBase *> &) final {
        return false;
    }

    bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                   std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                   std::vector<foeResourcePoolBase *> &resourcePools) {
        return false;
    }
    foeResourceCreateInfoBase *getResource(foeId id) final {
        return static_cast<foeResourceCreateInfoBase *>(mResReturn);
    }

    std::filesystem::path findExternalFile(std::filesystem::path externalFilePath) final {
        if (std::filesystem::exists(std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / mName /
                                    externalFilePath)) {
            return std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / mName / externalFilePath;
        }

        return {};
    }

  private:
    std::string mName;
    foeIdGroup mGroup;
    void *mResReturn;
};

TEST_CASE("foeGroupData - Initial State", "[foe]") {
    foeGroupData test;

    SECTION("Persistent Group") {
        SECTION("Resource Indices") {
            REQUIRE(test.persistentResourceIndices() != nullptr);
            REQUIRE(test.persistentResourceIndices() == test.resourceIndices(foeIdPersistentGroup));
            REQUIRE(test.persistentResourceIndices()->groupID() == foeIdPersistentGroup);
            REQUIRE(test.persistentResourceIndices() == test.resourceIndices("Persistent"));

            REQUIRE(test.resourceIndices(foeIdPersistentGroup) != nullptr);
            REQUIRE(test.resourceIndices(foeIdPersistentGroup) ==
                    test.resourceIndices(foeIdPersistentGroup));
            REQUIRE(test.resourceIndices(foeIdPersistentGroup)->groupID() == foeIdPersistentGroup);
            REQUIRE(test.resourceIndices(foeIdPersistentGroup) ==
                    test.resourceIndices("Persistent"));
        }

        SECTION("Entity Indices") {
            REQUIRE(test.persistentEntityIndices() != nullptr);
            REQUIRE(test.persistentEntityIndices() == test.entityIndices(foeIdPersistentGroup));
            REQUIRE(test.persistentEntityIndices()->groupID() == foeIdPersistentGroup);
            REQUIRE(test.persistentEntityIndices() == test.entityIndices("Persistent"));

            REQUIRE(test.entityIndices(foeIdPersistentGroup) != nullptr);
            REQUIRE(test.entityIndices(foeIdPersistentGroup) ==
                    test.entityIndices(foeIdPersistentGroup));
            REQUIRE(test.entityIndices(foeIdPersistentGroup)->groupID() == foeIdPersistentGroup);
            REQUIRE(test.entityIndices(foeIdPersistentGroup) == test.entityIndices("Persistent"));
        }

        SECTION("Importer") {
            REQUIRE(test.persistentImporter() == nullptr);
            REQUIRE(test.persistentImporter() == test.importer(foeIdPersistentGroup));
            REQUIRE(test.persistentImporter() == test.importer("Persistent"));

            REQUIRE(test.importer(foeIdPersistentGroup) == nullptr);
        }
    }

    SECTION("Temporary Group") {
        SECTION("Resource Indices") {
            REQUIRE(test.temporaryResourceIndices() != nullptr);
            REQUIRE(test.temporaryResourceIndices() == test.resourceIndices(foeIdTemporaryGroup));
            REQUIRE(test.temporaryResourceIndices()->groupID() == foeIdTemporaryGroup);
            REQUIRE(test.temporaryResourceIndices() == test.resourceIndices("Temporary"));

            REQUIRE(test.resourceIndices(foeIdTemporaryGroup) != nullptr);
            REQUIRE(test.resourceIndices(foeIdTemporaryGroup) ==
                    test.resourceIndices(foeIdTemporaryGroup));
            REQUIRE(test.resourceIndices(foeIdTemporaryGroup)->groupID() == foeIdTemporaryGroup);
            REQUIRE(test.resourceIndices(foeIdTemporaryGroup) == test.resourceIndices("Temporary"));
        }

        SECTION("Entity Indices") {
            REQUIRE(test.temporaryEntityIndices() != nullptr);
            REQUIRE(test.temporaryEntityIndices() == test.entityIndices(foeIdTemporaryGroup));
            REQUIRE(test.temporaryEntityIndices()->groupID() == foeIdTemporaryGroup);
            REQUIRE(test.temporaryEntityIndices() == test.entityIndices("Temporary"));

            REQUIRE(test.entityIndices(foeIdTemporaryGroup) != nullptr);
            REQUIRE(test.entityIndices(foeIdTemporaryGroup) ==
                    test.entityIndices(foeIdTemporaryGroup));
            REQUIRE(test.entityIndices(foeIdTemporaryGroup)->groupID() == foeIdTemporaryGroup);
            REQUIRE(test.entityIndices(foeIdTemporaryGroup) == test.entityIndices("Temporary"));
        }

        SECTION("Never an importer for the Temporary Group") {
            REQUIRE(test.importer(foeIdTemporaryGroup) == nullptr);
        }
    }

    SECTION("Dynamic Groups are all empty/nullptr") {
        for (uint32_t i = 0; i < (foeIdGroupMaxValue - 2); ++i) {
            REQUIRE(test.resourceIndices(foeIdValueToGroup(i)) == nullptr);
            REQUIRE(test.entityIndices(foeIdValueToGroup(i)) == nullptr);
            REQUIRE(test.importer(foeIdValueToGroup(i)) == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - addDynamicGroup failure cases", "[foe]") {
    foeGroupData test;

    SECTION("No EntityIndices given") {
        std::unique_ptr<foeIdIndexGenerator> testEntityIndices;
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("No ResourceIndices given") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        std::unique_ptr<foeIdIndexGenerator> testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("No Importer given") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        std::unique_ptr<DummyImporter> testImporter;

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Blank name in the importer") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Inconsistent ID Groups") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Persistent' ID Group") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdPersistentGroup);
        auto testResourceIndices = std::make_unique<foeIdIndexGenerator>("", foeIdPersistentGroup);
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdPersistentGroup);

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Persistent' reserved name") {
        auto testEntityIndices =
            std::make_unique<foeIdIndexGenerator>("Persistent", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("Persistent", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("Persistent", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Temporary' ID Group") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdTemporaryGroup);
        auto testResourceIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdTemporaryGroup);
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdTemporaryGroup);

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Temporary' reserved name") {
        auto testEntityIndices =
            std::make_unique<foeIdIndexGenerator>("Temporary", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("Temporary", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("Temporary", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Adding the same ID group more than once fails") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("Temporary", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(test.addDynamicGroup(std::move(testEntityIndices), std::move(testResourceIndices),
                                     std::move(testImporter)));

        testEntityIndices = std::make_unique<foeIdIndexGenerator>("b", foeIdValueToGroup(0x1));
        testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("Temporary", foeIdValueToGroup(0x1));
        testImporter = std::make_unique<DummyImporter>("b", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }

    SECTION("Adding the same Importer group name more than once fails") {
        auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testResourceIndices =
            std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(test.addDynamicGroup(std::move(testEntityIndices), std::move(testResourceIndices),
                                     std::move(testImporter)));

        testEntityIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x2));
        testResourceIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x2));
        testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(test.addDynamicGroup(
            std::move(testEntityIndices), std::move(testResourceIndices), std::move(testImporter)));
    }
}

TEST_CASE("foeGroupData - setPersistentImporter failure cases", "[foe]") {
    foeGroupData test;

    SECTION("Giving a nullptr") { REQUIRE_FALSE(test.setPersistentImporter(nullptr)); }

    SECTION("Giving an importer that is not foeIdPersistentGroup") {
        SECTION("Dynamic Groups") {
            for (foeIdGroup group = 0; group < foeIdNumDynamicGroups; ++group) {
                auto tempImporter = std::make_unique<DummyImporter>("testPersistent", group);

                REQUIRE_FALSE(test.setPersistentImporter(std::move(tempImporter)));
            }
        }

        SECTION("foeIdTemporaryGroup") {
            auto tempImporter =
                std::make_unique<DummyImporter>("testPersistent", foeIdTemporaryGroup);
            REQUIRE_FALSE(test.setPersistentImporter(std::move(tempImporter)));
        }
    }
}

TEST_CASE("foeGroupData - setPersistentImporter success cases", "[foe]") {
    foeGroupData test;

    auto tempImporter =
        std::make_unique<DummyImporter>("testPersistentImporter", foeIdPersistentGroup);
    auto *pImporter = tempImporter.get();

    REQUIRE(test.setPersistentImporter(std::move(tempImporter)));

    REQUIRE(test.importer(foeIdPersistentGroup) == pImporter);

    SECTION("Persistent importer with a different importer name also works") {
        REQUIRE(test.importer("testPersistentImporter") == pImporter);
    }
}

TEST_CASE("foeGroupData - IndexGenerator/Importer retrieval", "[foe]") {
    foeGroupData test;

    auto testEntityIndices = std::make_unique<foeIdIndexGenerator>("0x1", foeIdValueToGroup(0x1));
    auto testResourceIndices = std::make_unique<foeIdIndexGenerator>("0x1", foeIdValueToGroup(0x1));
    auto testImporter = std::make_unique<DummyImporter>("0x1", foeIdValueToGroup(0x1));
    REQUIRE(test.addDynamicGroup(std::move(testEntityIndices), std::move(testResourceIndices),
                                 std::move(testImporter)));

    SECTION("0x1 Group Data") {
        REQUIRE(test.resourceIndices(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.resourceIndices("0x1") != nullptr);

        REQUIRE(test.entityIndices(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.entityIndices("0x1") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.importer("0x1") != nullptr);
    }

    testEntityIndices = std::make_unique<foeIdIndexGenerator>(
        "MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    testResourceIndices = std::make_unique<foeIdIndexGenerator>(
        "MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    testImporter =
        std::make_unique<DummyImporter>("MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    REQUIRE(test.addDynamicGroup(std::move(testEntityIndices), std::move(testResourceIndices),
                                 std::move(testImporter)));

    SECTION("MaxDynamic Group Data") {
        REQUIRE(test.resourceIndices(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.resourceIndices("MaxDynamic") != nullptr);

        REQUIRE(test.entityIndices(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.entityIndices("MaxDynamic") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.importer("MaxDynamic") != nullptr);
    }

    SECTION("Failure cases") {
        SECTION("Temporary importer by name fails") {
            REQUIRE(test.importer("Temporary") == nullptr);
        }

        SECTION("Name not added") {
            REQUIRE(test.resourceIndices("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.resourceIndices("foeIdNumDynamicGroups") == nullptr);

            REQUIRE(test.entityIndices("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.entityIndices("foeIdNumDynamicGroups") == nullptr);

            REQUIRE(test.importer("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.importer("foeIdNumDynamicGroups") == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - getResourceDefinition", "[foe]") {
    foeGroupData test;
    foeResourceCreateInfoBase *pCreateInfo;

    SECTION("No importers always fails") {
        REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
    }

    SECTION("Persistent importer added") {
        SECTION("Returning true") {
            auto persistentImporter = std::make_unique<DummyImporter>(
                "testPersistent", foeIdPersistentGroup, static_cast<int *>(NULL) + 1);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            auto persistentImporter =
                std::make_unique<DummyImporter>("testPersistent", foeIdPersistentGroup);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
        }
    }

    SECTION("Dynamic importer added") {
        SECTION("Returning true") {
            auto testEntityIndices =
                std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testResourceIndices =
                std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1),
                                                                static_cast<int *>(NULL) + 1);

            REQUIRE(test.addDynamicGroup(std::move(testEntityIndices),
                                         std::move(testResourceIndices), std::move(testImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            auto testEntityIndices =
                std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testResourceIndices =
                std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

            REQUIRE(test.addDynamicGroup(std::move(testEntityIndices),
                                         std::move(testResourceIndices), std::move(testImporter)));
            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - Finding external files", "[foe]") {
    foeGroupData test;

    // Add a dynamic group
    auto testEntityIndices =
        std::make_unique<foeIdIndexGenerator>("dynamic", foeIdValueToGroup(0x1));
    auto testResourceIndices =
        std::make_unique<foeIdIndexGenerator>("dynamic", foeIdValueToGroup(0x1));
    auto testImporter = std::make_unique<DummyImporter>("dynamic", foeIdValueToGroup(0x1));

    REQUIRE(test.addDynamicGroup(std::move(testEntityIndices), std::move(testResourceIndices),
                                 std::move(testImporter)));

    SECTION("Only dynamic group importer set") {
        SECTION("Dynamic file is found in the dynamic location") {
            REQUIRE(test.findExternalFile("testDynamic.txt") ==
                    std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / "dynamic" /
                        "testDynamic.txt");
        }
        SECTION("Common file is found in the dynamic location") {
            REQUIRE(test.findExternalFile("common.txt") ==
                    std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / "dynamic" / "common.txt");
        }
        SECTION("Persistent file is not found") {
            REQUIRE(test.findExternalFile("testPersistent.txt").empty());
        }
    }

    auto persistentImporter = std::make_unique<DummyImporter>("persistent", foeIdPersistentGroup,
                                                              static_cast<int *>(NULL) + 1);
    REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

    SECTION("With both the dynamic and persistent importer set") {
        SECTION("Dynamic file is found in the dynamic location") {
            REQUIRE(test.findExternalFile("testDynamic.txt") ==
                    std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / "dynamic" /
                        "testDynamic.txt");
        }
        SECTION("Common file is found in the overridden persistent location") {
            REQUIRE(test.findExternalFile("common.txt") ==
                    std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / "persistent" /
                        "common.txt");
        }
        SECTION("Persistent file is not found") {
            REQUIRE(test.findExternalFile("testPersistent.txt") ==
                    std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / "persistent" /
                        "testPersistent.txt");
        }
    }
}
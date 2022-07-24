// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/imex/importer.h>
#include <foe/imex/result.h>
#include <foe/imex/type_defs.h>

TEST_CASE("foeImexImporter - When importer calls struct not available") {
    foeBaseInStructure dummyStruct = {};
    foeImexImporter emptyImporter = reinterpret_cast<foeImexImporter>(&dummyStruct);

    CHECK(foeImexImporterGetGroupID(emptyImporter, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetGroupName(emptyImporter, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterSetGroupTranslator(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetDependencies(emptyImporter, nullptr, nullptr, nullptr, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetGroupEntityIndexData(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetGroupResourceIndexData(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetStateData(emptyImporter, FOE_NULL_HANDLE, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetResourceDefinitions(emptyImporter, FOE_NULL_HANDLE, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetResourceEditorName(emptyImporter, FOE_INVALID_ID, nullptr, nullptr)
              .value == FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterGetResourceCreateInfo(emptyImporter, FOE_INVALID_ID, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);

    CHECK(foeImexImporterFindExternalFile(emptyImporter, nullptr, nullptr, nullptr).value ==
          FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

TEST_CASE("foeImexImporter - When importer calls struct available but no functions set") {
    foeImexImporterCalls emptyCalls{.sType = FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS};
    foeBaseInStructure dummyStruct = {.pNext = (foeBaseInStructure const *)&emptyCalls};
    foeImexImporter emptyImporter = reinterpret_cast<foeImexImporter>(&dummyStruct);

    CHECK(foeImexImporterGetGroupID(emptyImporter, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetGroupName(emptyImporter, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterSetGroupTranslator(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetDependencies(emptyImporter, nullptr, nullptr, nullptr, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetGroupEntityIndexData(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetGroupResourceIndexData(emptyImporter, FOE_NULL_HANDLE).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetStateData(emptyImporter, FOE_NULL_HANDLE, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetResourceDefinitions(emptyImporter, FOE_NULL_HANDLE, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetResourceEditorName(emptyImporter, FOE_INVALID_ID, nullptr, nullptr)
              .value == FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterGetResourceCreateInfo(emptyImporter, FOE_INVALID_ID, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);

    CHECK(foeImexImporterFindExternalFile(emptyImporter, nullptr, nullptr, nullptr).value ==
          FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
}
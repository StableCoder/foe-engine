#include "import_state.hpp"

#include <foe/log.hpp>
#include <foe/search_paths.hpp>

#include "../simulation_set.hpp"
#include "error_code.hpp"
#include "importer_base.hpp"

bool foeImportState::addImporterGenerator(
    foeImporterBase *(*pGeneratorFunction)(foeIdGroup, std::filesystem::path)) {

    for (auto it : mImporterGenerators) {
        if (it == pGeneratorFunction)
            return false;
    }

    mImporterGenerators.emplace_back(pGeneratorFunction);
    return true;
}

void foeImportState::removeImporterGenerator(
    foeImporterBase *(*pGeneratorFunction)(foeIdGroup, std::filesystem::path)) {

    for (auto it = mImporterGenerators.begin(); it != mImporterGenerators.end(); ++it) {
        if (*it == pGeneratorFunction)
            it = mImporterGenerators.erase(it);
    }
}

namespace {

bool generateDependencyImporters(
    std::vector<std::string> const &dependencies,
    foeSearchPaths *pSearchPaths,
    std::vector<foeImporterBase *(*)(foeIdGroup, std::filesystem::path)> const &importerGenerators,
    std::vector<std::unique_ptr<foeImporterBase>> &importers) {
    std::vector<std::unique_ptr<foeImporterBase>> newImporters;

    auto pathReader = pSearchPaths->getReader();

    foeIdGroup groupValue = 0;
    for (auto const &depIt : dependencies) {
        for (auto searchPath : *pathReader.searchPaths()) {
            for (auto dirIt : std::filesystem::directory_iterator{searchPath}) {
                auto path = dirIt.path();

                if (path.stem() == depIt) {
                    foeImporterBase *pImporter{nullptr};
                    for (auto it : importerGenerators) {
                        pImporter = it(foeIdValueToGroup(groupValue), path);
                        if (pImporter != nullptr) {
                            newImporters.emplace_back(pImporter);
                            ++groupValue;
                            goto DEPENDENCY_FOUND;
                        }
                    }
                    if (pImporter == nullptr) {
                        FOE_LOG(General, Error, "Failed to find importer for dependency at: {}",
                                path.string())
                        return false;
                    }
                }
            }
        }
    DEPENDENCY_FOUND:;
    }

    importers = std::move(newImporters);
    return true;
}

} // namespace

auto foeImportState::importState(std::filesystem::path stateDataPath,
                                 foeSearchPaths *pSearchPaths,
                                 SimulationSet **ppSimulationSet) -> std::error_code {
    auto pSimulationSet = std::make_unique<SimulationSet>();

    // Find the importer for the starting path
    std::unique_ptr<foeImporterBase> persistentImporter;
    for (auto it : mImporterGenerators) {
        auto *pImporter = it(foeIdPersistentGroup, stateDataPath);
        if (pImporter != nullptr) {
            persistentImporter.reset(pImporter);
            break;
        }
    }
    if (persistentImporter == nullptr) {
        FOE_LOG(General, Error, "Could not find importer for state data at path: {}",
                stateDataPath.string())
        return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;
    }

    // Get the list of dependencies
    std::vector<std::string> dependencies;
    bool pass = persistentImporter->getDependencies(dependencies);
    if (!pass)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;

    // Check for duplicate dependencies
    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
        for (auto innerIt = it + 1; innerIt != dependencies.end(); ++innerIt) {
            if (*innerIt == *it) {
                FOE_LOG(General, Error, "Duplicate dependency '{}' detected")
                return FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES;
            }
        }
    }

    // Generate importers for all of the dependencies
    std::vector<std::unique_ptr<foeImporterBase>> dependencyImporters;
    pass = generateDependencyImporters(dependencies, pSearchPaths, mImporterGenerators,
                                       dependencyImporters);
    if (!pass)
        return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;

    { // Check transitive dependencies
        auto pImporter = dependencyImporters.begin();
        for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ++depIt, ++pImporter) {
            std::vector<std::string> transitiveDependencies;
            pass = pImporter->get()->getDependencies(transitiveDependencies);
            if (!pass) {
                FOE_LOG(General, Error, "Failed to import sub-dependencies of the '{}' dependency",
                        *depIt)
                return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;
            }

            // Check that all required transitive dependencies are available *before* it is loaded,
            // and in the correct order
            auto checkIt = dependencies.begin();
            for (auto transIt : transitiveDependencies) {
                bool depFound{false};

                for (; checkIt != depIt; ++checkIt) {
                    if (*checkIt == transIt) {
                        depFound = true;
                        break;
                    }
                }

                if (!depFound) {
                    FOE_LOG(General, Error,
                            "Could not find transitive dependency '{}' for dependency group '{}'",
                            transIt, *depIt)
                    return FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED;
                }
            }
        }
    }

    // Setup importer translations
    for (auto const &it : dependencyImporters) {
        std::vector<std::string> groupDependencies;
        it->getDependencies(groupDependencies);

        foeGroupTranslation newTranslation;
        newTranslation.generateTranslations(groupDependencies, &pSimulationSet->groupData);

        it->setGroupTranslation(std::move(newTranslation));
    }

    // Persistent Group Index Data
    bool retVal =
        persistentImporter->getGroupIndexData(*pSimulationSet->groupData.persistentIndices());
    if (!retVal) {
        return FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA;
    }

    { // Setup the ECS Groups
        foeIdGroup groupValue = 0;
        for (auto &it : dependencyImporters) {
            std::string name{it->name()};
            auto newGroupIndices =
                std::make_unique<foeIdIndexGenerator>(name, foeIdValueToGroup(groupValue));

            auto success = pSimulationSet->groupData.addDynamicGroup(std::move(newGroupIndices),
                                                                     std::move(it));
            if (!success) {
                FOE_LOG(General, Error, "Could not setup Group '{}'", name);
                return FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE;
            }
            ++groupValue;
        }

        pSimulationSet->groupData.setPersistentImporter(std::move(persistentImporter));
    }

    // Load Persistent resource definitions

    // Load dependency resource definitions

    // Importing Dependency State Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdMaxDynamicGroups; ++groupValue) {
        auto *pGroupImporter = pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (pGroupImporter != nullptr) {
            pGroupImporter->importStateData(&pSimulationSet->state);
        }
    }

    // Importing Persistent State Data
    pSimulationSet->groupData.persistentImporter()->importStateData(&pSimulationSet->state);

    // Successfully returning
    *ppSimulationSet = pSimulationSet.release();
    return FOE_STATE_IMPORT_SUCCESS;
}
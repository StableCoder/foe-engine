#ifndef IMPORT_STATE_HPP
#define IMPORT_STATE_HPP

#include <foe/ecs/id.hpp>

#include <filesystem>
#include <functional>
#include <vector>

class foeImporterBase;
class foeSearchPaths;
struct SimulationSet;

class foeImportState {
  public:
    bool addImporterGenerator(foeImporterBase *(*pGeneratorFunction)(foeIdGroup,
                                                                     std::filesystem::path));
    void removeImporterGenerator(foeImporterBase *(*pGeneratorFunction)(foeIdGroup,
                                                                        std::filesystem::path));

    auto importState(std::filesystem::path stateDataPath,
                     foeSearchPaths *pSearchPaths,
                     SimulationSet **ppSimulationSet) -> std::error_code;

  private:
    std::vector<foeImporterBase *(*)(foeIdGroup, std::filesystem::path)> mImporterGenerators;
};

#endif // IMPORT_STATE_HPP
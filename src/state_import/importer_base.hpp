#ifndef IMPORTER_BASE_HPP
#define IMPORTER_BASE_HPP

#include "group_translation.hpp"

#include <string>
#include <vector>

class foeIdIndexGenerator;
class foeEcsGroups;
struct StatePools;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual void setGroupTranslation(foeGroupTranslation &&groupTranslation) = 0;

    virtual bool getDependencies(std::vector<std::string> &dependencies) = 0;
    virtual bool getGroupIndexData(foeIdIndexGenerator &ecsGroup) = 0;
    virtual bool importStateData(foeEcsGroups *pGroups, StatePools *pStatePools) = 0;
};

#endif // IMPORTER_BASE_HPP
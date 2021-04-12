#ifndef IMPORTER_BASE_HPP
#define IMPORTER_BASE_HPP

#include "group_translation.hpp"

#include <string>
#include <vector>

class foeIdIndexGenerator;
struct StatePools;
struct foeResourceCreateInfoBase;

struct foeImporterDependencySet {
    std::string name;
    foeIdGroup groupValue;
};

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual void setGroupTranslation(foeGroupTranslation &&groupTranslation) = 0;

    virtual bool getDependencies(std::vector<foeImporterDependencySet> &dependencies) = 0;
    virtual bool getGroupIndexData(foeIdIndexGenerator &ecsGroup) = 0;
    virtual bool importStateData(StatePools *pStatePools) = 0;

    virtual bool getResource(foeId id, foeResourceCreateInfoBase **ppCreateInfo) = 0;
};

#endif // IMPORTER_BASE_HPP
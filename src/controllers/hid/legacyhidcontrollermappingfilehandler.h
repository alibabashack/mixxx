#pragma once

#include "controllers/legacycontrollermappingfilehandler.h"

class LegacyHidControllerMapping;

class LegacyHidControllerMappingFileHandler : public LegacyControllerMappingFileHandler {
  public:
    LegacyHidControllerMappingFileHandler(){};
    ~LegacyHidControllerMappingFileHandler() override{};

    [[nodiscard]] bool save(const LegacyHidControllerMapping& mapping, const QString& fileName) const;

  private:
    [[nodiscard]] std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingsPath) override;
};

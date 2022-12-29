#pragma once

#include <lilv/lilv.h>

#include "effects/backends/effectsbackend.h"
#include "effects/backends/lv2/lv2manifest.h"
#include "effects/defs.h"
#include "preferences/usersettings.h"

/// Refer to EffectsBackend for documentation
class LV2Backend : public EffectsBackend {
  public:
    LV2Backend();
    ~LV2Backend() override;

    EffectBackendType getType() const override {
        return EffectBackendType::LV2;
    };

    const QList<QString> getEffectIds() const override;
    const QSet<QString> getDiscoveredPluginIds() const;
    EffectManifestPointer getManifest(const QString& effectId) const override;
    const QList<EffectManifestPointer> getManifests() const override;
    LV2EffectManifestPointer getLV2Manifest(const QString& effectId) const;
    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override;
    bool canInstantiateEffect(const QString& effectId) const override;

  private:
    void enumeratePlugins();
    void initializeProperties();
    LilvWorld* m_pWorld;
    QHash<QString, LilvNode*> m_properties;
    QHash<QString, LV2EffectManifestPointer> m_registeredEffects;

    QString debugString() const {
        return "LV2Backend";
    }
};

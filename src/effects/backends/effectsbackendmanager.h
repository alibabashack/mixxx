#pragma once

#include "effects/backends/effectsbackend.h"

class ControlObject;

/// EffectsBackendManager initializes EffectsBackends, maintains the list of
/// available EffectManifests, and creates EffectProcessors from EffectManifests.
class EffectsBackendManager {
  public:
    EffectsBackendManager();
    ~EffectsBackendManager() = default;

    [[nodiscard]] const QList<EffectManifestPointer>& getManifests() const {
        return m_manifests;
    };
    [[nodiscard]] const QList<EffectManifestPointer> getManifestsForBackend(EffectBackendType backendType) const;
    [[nodiscard]] EffectManifestPointer getManifestFromUniqueId(const QString& uid) const;
    /// returns a pointer to the manifest or a null pointer in case a
    /// the previously stored backend or effect is no longer available
    [[nodiscard]] EffectManifestPointer getManifest(const QString& id, EffectBackendType backendType) const;
    [[nodiscard]] const QString getDisplayNameForEffectPreset(EffectPresetPointer pPreset) const;

    std::unique_ptr<EffectProcessor> createProcessor(const EffectManifestPointer pManifest);

  private:
    void addBackend(EffectsBackendPointer pEffectsBackend);

    std::unique_ptr<ControlObject> m_pNumEffectsAvailable;

    QHash<EffectBackendType, EffectsBackendPointer> m_effectsBackends;
    QList<EffectManifestPointer> m_manifests;
};

typedef QSharedPointer<EffectsBackendManager> EffectsBackendManagerPointer;

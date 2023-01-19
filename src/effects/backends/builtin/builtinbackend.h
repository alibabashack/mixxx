#pragma once

#include "effects/backends/effectsbackend.h"
#include "effects/defs.h"

/// Refer to EffectsBackend for documentation
class BuiltInBackend : public EffectsBackend {
  public:
    BuiltInBackend();
    ~BuiltInBackend() override;

    [[nodiscard]] EffectBackendType getType() const override {
        return EffectBackendType::BuiltIn;
    };

    [[nodiscard]] const QList<QString> getEffectIds() const override;
    [[nodiscard]] EffectManifestPointer getManifest(const QString& effectId) const override;
    [[nodiscard]] const QList<EffectManifestPointer> getManifests() const override;
    [[nodiscard]] std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override;
    [[nodiscard]] bool canInstantiateEffect(const QString& effectId) const override;

  private:
    [[nodiscard]] QString debugString() const {
        return "BuiltInBackend";
    }

    typedef std::unique_ptr<EffectProcessor> (*EffectProcessorInstantiator)();

    struct RegisteredEffect {
        EffectManifestPointer pManifest;
        EffectProcessorInstantiator instantiator;
    };

    void registerEffectInner(const QString& id,
            EffectManifestPointer pManifest,
            EffectProcessorInstantiator instantiator);

    template<typename EffectProcessorImpl>
    void registerEffect() {
        registerEffectInner(
                EffectProcessorImpl::getId(),
                EffectProcessorImpl::getManifest(),
                []() {
                    return static_cast<std::unique_ptr<EffectProcessor>>(
                            std::make_unique<EffectProcessorImpl>());
                });
    };

    QMap<QString, RegisteredEffect> m_registeredEffects;
    QList<QString> m_effectIds;
};

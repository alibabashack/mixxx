#include "effects/effectslot.h"

#include <QDebug>

#include "control/controlencoder.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "effects/defs.h"
#include "util/defs.h"
#include "util/math.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 16;

EffectSlot::EffectSlot(const QString& group,
                       EffectsManager* pEffectsManager,
                       const unsigned int iEffectnumber,
                       EngineEffectChain* pEngineEffectChain)
        : m_iEffectNumber(iEffectnumber),
          m_group(group),
          m_pEffectsManager(pEffectsManager),
          m_pEngineEffectChain(pEngineEffectChain),
          m_pEngineEffect(nullptr) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffectChain != nullptr) {
        return;
    }

    m_pControlLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->setReadOnly();

    m_pControlNumParameters.insert(EffectManifestParameter::ParameterType::KNOB,
            new ControlObject(ConfigKey(m_group, "num_parameters")));
    m_pControlNumParameters.insert(EffectManifestParameter::ParameterType::BUTTON,
            new ControlObject(ConfigKey(m_group, "num_button_parameters")));
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->setReadOnly();
    }

    m_pControlNumParameterSlots.insert(EffectManifestParameter::ParameterType::KNOB,
            new ControlObject(ConfigKey(m_group, "num_parameterslots")));
    m_pControlNumParameterSlots.insert(EffectManifestParameter::ParameterType::BUTTON,
            new ControlObject(ConfigKey(m_group, "num_button_parameterslots")));
    for (const auto& pControlNumParameterSlots : m_pControlNumParameterSlots) {
        pControlNumParameterSlots->setReadOnly();
    }

    // Default to disabled to prevent accidental activation of effects
    // at the beginning of a set.
    m_pControlEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    connect(m_pControlEnabled, &ControlObject::valueChanged,
            this, &EffectSlot::updateEngineState);

    m_pControlNextEffect = new ControlPushButton(ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect, &ControlObject::valueChanged,
            this, &EffectSlot::slotNextEffect);

    m_pControlPrevEffect = new ControlPushButton(ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect, &ControlObject::valueChanged,
            this, &EffectSlot::slotPrevEffect);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlEffectSelector = new ControlEncoder(ConfigKey(m_group, "effect_selector"), false);
    connect(m_pControlEffectSelector, &ControlObject::valueChanged,
            this, &EffectSlot::slotEffectSelector);

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, &ControlObject::valueChanged,
            this, &EffectSlot::slotClear);

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot(EffectManifestParameter::ParameterType::KNOB);
        addEffectParameterSlot(EffectManifestParameter::ParameterType::BUTTON);
    }

    m_pControlMetaParameter = new ControlPotmeter(ConfigKey(m_group, "meta"), 0.0, 1.0);
    // QObject::connect cannot connect to slots with optional parameters using function
    // pointer syntax if the slot has more parameters than the signal, so use a lambda
    // to hack around this limitation.
    connect(m_pControlMetaParameter, &ControlObject::valueChanged,
            this, [=](double value){slotEffectMetaParameter(value);} );
    m_pControlMetaParameter->set(0.0);
    m_pControlMetaParameter->setDefaultValue(0.0);

    m_pMetaknobSoftTakeover = new SoftTakeover();

    m_pControlLoaded->forceSet(0.0);
}

EffectSlot::~EffectSlot() {
    //qDebug() << debugString() << "destroyed";
    unloadEffect();

    delete m_pControlLoaded;
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        delete pControlNumParameters;
    }
    for (const auto& pControlNumParameterSlots : m_pControlNumParameterSlots) {
        delete pControlNumParameterSlots;
    }
    delete m_pControlNextEffect;
    delete m_pControlPrevEffect;
    delete m_pControlEffectSelector;
    delete m_pControlClear;
    delete m_pControlEnabled;
    delete m_pControlMetaParameter;
    delete m_pMetaknobSoftTakeover;
}

void EffectSlot::addToEngine(std::unique_ptr<EffectProcessor> pProcessor,
        const QSet<ChannelHandleAndGroup>& activeInputChannels) {
    VERIFY_OR_DEBUG_ASSERT(!isLoaded()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect == nullptr) {
        return;
    }

    m_pEngineEffect = new EngineEffect(m_pManifest,
            activeInputChannels,
            m_pEffectsManager,
            std::move(pProcessor));

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = m_iEffectNumber;
    m_pEffectsManager->writeRequest(request);
}

void EffectSlot::removeFromEngine() {
    VERIFY_OR_DEBUG_ASSERT(isLoaded()) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = m_iEffectNumber;
    m_pEffectsManager->writeRequest(request);

    m_pEngineEffect = nullptr;
}

void EffectSlot::updateEngineState() {
    if (!m_pEngineEffect) {
        return;
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetEffectParameters.enabled = m_pControlEnabled->get();
    m_pEffectsManager->writeRequest(pRequest);

    for (const auto& parameterList : m_parameters) {
        for (auto const& pParameter : parameterList) {
            pParameter->updateEngineState();
        }
    }
}

void EffectSlot::fillEffectStatesMap(EffectStatesMap* pStatesMap) const {
    //TODO: get actual configuration of engine
    const mixxx::EngineParameters bufferParameters(
            mixxx::AudioSignal::SampleRate(96000),
            MAX_BUFFER_LEN / mixxx::kEngineChannelCount);

    if (isLoaded()) {
        for (const auto& outputChannel : m_pEffectsManager->registeredOutputChannels()) {
            pStatesMap->insert(outputChannel.handle(),
                    m_pEngineEffect->createState(bufferParameters));
        }
    } else {
        for (EffectState* pState : *pStatesMap) {
            if (pState != nullptr) {
                delete pState;
            }
        }
        pStatesMap->clear();
    }
};

EffectManifestPointer EffectSlot::getManifest() const {
    return m_pManifest;
}

void EffectSlot::addEffectParameterSlot(EffectManifestParameter::ParameterType parameterType) {
    EffectParameterSlotBasePointer pParameterSlot = EffectParameterSlotBasePointer();
    if (parameterType == EffectManifestParameter::ParameterType::KNOB) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectKnobParameterSlot(m_group, m_iNumParameterSlots[parameterType]));
    } else if (parameterType == EffectManifestParameter::ParameterType::BUTTON) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectButtonParameterSlot(m_group, m_iNumParameterSlots[parameterType]));
    }
    ++m_iNumParameterSlots[parameterType];
    m_pControlNumParameterSlots[parameterType]->forceSet(
            m_pControlNumParameterSlots[parameterType]->get() + 1);
    VERIFY_OR_DEBUG_ASSERT(m_iNumParameterSlots[parameterType] ==
            m_pControlNumParameterSlots[parameterType]->get()) {
        return;
    }
    m_parameterSlots[parameterType].append(pParameterSlot);
}

unsigned int EffectSlot::numParameters(EffectManifestParameter::ParameterType parameterType) const {
    return m_parameters.value(parameterType).size();
}

void EffectSlot::setEnabled(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotBasePointer EffectSlot::getEffectParameterSlot(
        EffectManifestParameter::ParameterType parameterType,
        unsigned int slotNumber) {
    VERIFY_OR_DEBUG_ASSERT(slotNumber <= (unsigned)m_parameterSlots.value(parameterType).size()) {
        return nullptr;
    }
    return m_parameterSlots.value(parameterType).at(slotNumber);
}

void EffectSlot::loadEffect(const EffectManifestPointer pManifest,
        std::unique_ptr<EffectProcessor> pProcessor,
        EffectPresetPointer pEffectPreset,
        const QSet<ChannelHandleAndGroup>& activeChannels,
        bool adoptMetaknobFromPreset) {
    if (kEffectDebugOutput) {
        if (pManifest != nullptr) {
            qDebug() << this << m_group << "loading effect" << pManifest->id() << pEffectPreset.get() << pProcessor.get();
        } else {
            qDebug() << this << m_group << "unloading effect";
        }
    }
    unloadEffect();

    m_pManifest = pManifest;

    if (pManifest == EffectManifestPointer()) {
        // No new effect to load; just unload the old effect and return.
        emit effectChanged();
        return;
    }

    addToEngine(std::move(pProcessor), activeChannels);

    // Create EffectParameters. Every parameter listed in the manifest must have
    // an EffectParameter created, regardless of whether it is loaded in a slot.
    int manifestIndex = 0;
    for (const auto& pManifestParameter: m_pManifest->parameters()) {
        // match the manifest parameter to the preset parameter
        EffectParameterPreset parameterPreset = EffectParameterPreset();
        if (pEffectPreset != nullptr) {
            for (const auto& p : pEffectPreset->getParameterPresets()) {
                if (p.id() == pManifestParameter->id()) {
                    parameterPreset = p;
                }
            }
        }
        EffectParameterPointer pParameter(new EffectParameter(
                m_pEngineEffect, m_pEffectsManager, manifestIndex, pManifestParameter, parameterPreset));
        m_parameters[pManifestParameter->parameterType()].append(pParameter);
        manifestIndex++;
    }

    // Map the parameter slots to the EffectParameters.
    // The slot order is determined by the order parameters are listed in the preset.
    int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
    for (int parameterTypeId = 0; parameterTypeId < numTypes; ++parameterTypeId) {
        const EffectManifestParameter::ParameterType parameterType =
                static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

        if (pEffectPreset != nullptr && !pEffectPreset.isNull()) {
            m_loadedParameters[parameterType].clear();
            int slot = 0;
            for (const auto& parameterPreset : pEffectPreset->getParameterPresets()) {
                if (parameterPreset.hidden() || parameterPreset.isNull()) {
                    continue;
                }

                for (const auto& pParameter : m_parameters.value(parameterType)) {
                    if (pParameter->manifest()->id() == parameterPreset.id()) {
                        m_loadedParameters[parameterType].insert(slot, pParameter);
                        break;
                    }
                }
                slot++;
            }
        }
    }

    loadParameters();

    m_pControlLoaded->forceSet(1.0);

    if (m_pEffectsManager->isAdoptMetaknobValueEnabled()) {
        if (adoptMetaknobFromPreset) {
            // Update the ControlObject value, but do not sync the parameters
            // with slotEffectMetaParameter. This allows presets to intentionally
            // save parameters in a state inconsistent with the metaknob.
            m_pControlMetaParameter->set(pEffectPreset->metaParameter());
        } else {
            slotEffectMetaParameter(m_pControlMetaParameter->get(), true);
        }
    } else {
        m_pControlMetaParameter->set(pEffectPreset->metaParameter());
        slotEffectMetaParameter(pEffectPreset->metaParameter(), true);
    }

    emit effectChanged();
    updateEngineState();
}

void EffectSlot::unloadEffect() {
    if (!isLoaded()) {
        return;
    }

    m_pControlLoaded->forceSet(0.0);
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->forceSet(0.0);
    }

    for (auto& slotList : m_parameterSlots) {
        // Do not delete the slots; clear the parameters from the slots
        // The parameter slots are used by the next effect, but the EffectParameters
        // are deleted below.
        for (auto pSlot : slotList) {
            pSlot->clear();
        }
    }
    for (auto& parameterList : m_parameters) {
        parameterList.clear();
    }
    for (auto& parameterList : m_loadedParameters) {
        parameterList.clear();
    }

    m_pManifest.clear();

    removeFromEngine();
}

void EffectSlot::loadParameters() {
    //qDebug() << this << m_group << "loading parameters";
    int numTypes = static_cast<int>(EffectManifestParameter::ParameterType::NUM_TYPES);
    for (int parameterTypeId=0 ; parameterTypeId<numTypes ; ++parameterTypeId) {
        const EffectManifestParameter::ParameterType parameterType =
                static_cast<EffectManifestParameter::ParameterType>(parameterTypeId);

        m_pControlNumParameters[parameterType]->forceSet(numParameters(parameterType));

        int slot = 0;
        for (auto pParameter : m_loadedParameters.value(parameterType)) {
            // LV2 effects may have more parameters than there are slots available
            if ((unsigned)slot >= kDefaultMaxParameters) {
                return;
            }
            VERIFY_OR_DEBUG_ASSERT(slot <= m_parameterSlots.value(parameterType).size()) {
                break;
            }
            m_parameterSlots.value(parameterType).at(slot)->loadParameter(pParameter);
            slot++;
        }

        // Clear any EffectParameterSlots that still have a loaded parameter from before
        // but the loop above did not load a new parameter into them.
        for (; slot < m_parameterSlots.value(parameterType).size(); slot++) {
            m_parameterSlots.value(parameterType).at(slot)->clear();
        }
    }
}

void EffectSlot::hideParameter(EffectParameterPointer pParameter) {
    auto parameterType = pParameter->manifest()->parameterType();
    VERIFY_OR_DEBUG_ASSERT(m_parameters.value(parameterType).contains(pParameter)) {
        return;
    }
    m_loadedParameters[parameterType].removeAll(pParameter);
    loadParameters();
}

void EffectSlot::showParameter(EffectParameterPointer pParameter) {
    auto parameterType = pParameter->manifest()->parameterType();
    VERIFY_OR_DEBUG_ASSERT(m_parameters.value(parameterType).contains(pParameter)) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(!m_loadedParameters.value(parameterType).contains(pParameter)) {
        return;
    }
    m_loadedParameters[parameterType].append(pParameter);
    loadParameters();
}

void EffectSlot::slotPrevEffect(double v) {
    if (v > 0) {
        slotEffectSelector(-1);
    }
}

void EffectSlot::slotNextEffect(double v) {
    if (v > 0) {
        slotEffectSelector(1);
    }
}

void EffectSlot::slotEffectSelector(double v) {
    // TODO: reimplement
    // if (v > 0) {
    //     emit nextEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    // } else if (v < 0) {
    //     emit prevEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    // }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        unloadEffect();
        emit effectChanged();
    }
}

void EffectSlot::syncSofttakeover() {
    for (const auto& parameterSlotList : m_parameterSlots) {
        for (const auto& pParameterSlot : parameterSlotList) {
            if (pParameterSlot->parameterType() == EffectManifestParameter::ParameterType::KNOB) {
                pParameterSlot->syncSofttakeover();
            }
        }
    }
}

double EffectSlot::getMetaParameter() const {
    return m_pControlMetaParameter->get();
}

// This function is for the superknob to update individual effects' meta knobs
// slotEffectMetaParameter does not need to update m_pControlMetaParameter's value
void EffectSlot::setMetaParameter(double v, bool force) {
    if (!m_pMetaknobSoftTakeover->ignore(m_pControlMetaParameter, v)
            || !m_pControlEnabled->toBool()
            || force) {
        m_pControlMetaParameter->set(v);
        slotEffectMetaParameter(v, force);
    }
}

void EffectSlot::slotEffectMetaParameter(double v, bool force) {
    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlMetaParameter->set(v);
    }
    if (!m_pControlEnabled->toBool()) {
        force = true;
    }

    // Only knobs are linked to the metaknob; not buttons
    for (const auto& pParameterSlot : m_parameterSlots.value(EffectManifestParameter::ParameterType::KNOB)) {
        if (pParameterSlot->parameterType() == EffectManifestParameter::ParameterType::KNOB) {
            pParameterSlot->onEffectMetaParameterChanged(v, force);
        }
    }
}

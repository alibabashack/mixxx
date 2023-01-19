#include "engine/channels/engineaux.h"

#include "control/control.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "moc_engineaux.cpp"
#include "preferences/usersettings.h"
#include "util/sample.h"

EngineAux::EngineAux(const ChannelHandleAndGroup& handleGroup, EffectsManager* pEffectsManager)
        : EngineChannel(handleGroup, EngineChannel::CENTER, pEffectsManager,
                  /*isTalkoverChannel*/ false,
                  /*isPrimaryDeck*/ false),
          m_inputConfigured(ConfigKey(getGroup(), "input_configured")),
          m_pregain(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5) {
    // Make input_configured read-only.
    m_inputConfigured.setReadOnly();
    ControlDoublePrivate::insertAlias(ConfigKey(getGroup(), "enabled"),
                                      ConfigKey(getGroup(), "input_configured"));

    // by default Aux is disabled on the master and disabled on PFL. User
    // can over-ride by setting the "pfl" or "master" controls.
    // Skins can change that during initialisation, if the master control is not provided.
    setMaster(false);
}

EngineChannel::ActiveState EngineAux::updateActiveState() {
    bool enabled = m_inputConfigured.toBool();
    if (enabled && m_sampleBuffer) {
        m_active = true;
        return ActiveState::Active;
    }
    if (m_active) {
        m_vuMeter.reset();
        m_active = false;
        return ActiveState::WasActive;
    }
    return ActiveState::Inactive;
}

void EngineAux::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_inputConfigured.forceSet(1.0);
}

void EngineAux::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_inputConfigured.forceSet(0.0);
}

void EngineAux::receiveBuffer(
        const AudioInput& input, const CSAMPLE* pBuffer, unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    m_sampleBuffer = pBuffer;
}

void EngineAux::process(CSAMPLE* pOut, const int iBufferSize) {
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    CSAMPLE_GAIN pregain = static_cast<CSAMPLE_GAIN>(m_pregain.get());
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
        EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
        if (pEngineEffectsManager != nullptr) {
            pEngineEffectsManager->processPreFaderInPlace(
                    m_group.handle(), m_pEffectsManager->getMasterHandle(), pOut, iBufferSize,
                    // TODO(jholthuis): Use mixxx::audio::SampleRate instead
                    static_cast<unsigned int>(m_sampleRate.get()));
        }
        m_sampleBuffer = nullptr;
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineAux::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_vuMeter.collectFeatures(pGroupFeatures);
}

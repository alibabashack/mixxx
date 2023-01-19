#include "engine/channels/enginechannel.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_enginechannel.cpp"

EngineChannel::EngineChannel(const ChannelHandleAndGroup& handleGroup,
        EngineChannel::ChannelOrientation defaultOrientation,
        EffectsManager* pEffectsManager,
        bool isTalkoverChannel,
        bool isPrimaryDeck)
        : m_group(handleGroup),
          m_pEffectsManager(pEffectsManager),
          m_vuMeter(getGroup()),
          m_sampleRate("[Master]", "samplerate"),
          m_sampleBuffer(nullptr),
          m_bIsPrimaryDeck(isPrimaryDeck),
          m_active(false),
          m_master(ConfigKey(getGroup(), "master")),
          m_PFL(ConfigKey(getGroup(), "pfl")),
          m_orientation(ConfigKey(getGroup(), "orientation")),
          m_orientationLeft(ConfigKey(getGroup(), "orientation_left")),
          m_orientationRight(ConfigKey(getGroup(), "orientation_right")),
          m_orientationCenter(ConfigKey(getGroup(), "orientation_center")),
          m_talkover(ConfigKey(getGroup(), "talkover")),
          m_bIsTalkoverChannel(isTalkoverChannel),
          m_channelIndex(-1) {
    m_PFL.setButtonMode(ControlPushButton::TOGGLE);
    m_master.setButtonMode(ControlPushButton::POWERWINDOW);
    m_orientation.setButtonMode(ControlPushButton::TOGGLE);
    m_orientation.setStates(3);
    m_orientation.set(defaultOrientation);
    connect(&m_orientationLeft, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationLeft, Qt::DirectConnection);
    connect(&m_orientationRight, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationRight, Qt::DirectConnection);
    connect(&m_orientationCenter, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationCenter, Qt::DirectConnection);
    m_talkover.setButtonMode(ControlPushButton::POWERWINDOW);

    if (m_pEffectsManager != nullptr) {
        m_pEffectsManager->registerInputChannel(handleGroup);
    }
}

void EngineChannel::setPfl(bool enabled) {
    m_PFL.set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isPflEnabled() const {
    return m_PFL.toBool();
}

void EngineChannel::setMaster(bool enabled) {
    m_master.set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isMasterEnabled() const {
    return m_master.toBool();
}

void EngineChannel::setTalkover(bool enabled) {
    m_talkover.set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isTalkoverEnabled() const {
    return m_talkover.toBool();
}

void EngineChannel::slotOrientationLeft(double v) {
    if (v > 0) {
        m_orientation.set(LEFT);
    }
}

void EngineChannel::slotOrientationRight(double v) {
    if (v > 0) {
        m_orientation.set(RIGHT);
    }
}

void EngineChannel::slotOrientationCenter(double v) {
    if (v > 0) {
        m_orientation.set(CENTER);
    }
}

EngineChannel::ChannelOrientation EngineChannel::getOrientation() const {
    const double dOrientation = m_orientation.get();
    const int iOrientation = static_cast<int>(dOrientation);
    if (dOrientation != iOrientation) {
        return CENTER;
    }
    switch (iOrientation) {
    case LEFT:
        return LEFT;
    case CENTER:
        return CENTER;
    case RIGHT:
        return RIGHT;
    default:
        return CENTER;
    }
}

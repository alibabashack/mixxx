#pragma once

#include "control/controlpushbutton.h"
#include "control/pollingcontrolproxy.h"
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/engineobject.h"
#include "engine/enginevumeter.h"
#include "preferences/usersettings.h"

class ControlObject;
class EngineBuffer;

class EngineChannel : public EngineObject {
    Q_OBJECT
  public:
    enum ChannelOrientation {
        LEFT = 0,
        CENTER,
        RIGHT,
    };

    enum class ActiveState {
        Inactive = 0,
        Active,
        WasActive
    };

    EngineChannel(const ChannelHandleAndGroup& handleGroup,
            ChannelOrientation defaultOrientation,
            EffectsManager* pEffectsManager,
            bool isTalkoverChannel,
            bool isPrimaryDeck);

    [[nodiscard]] virtual ChannelOrientation getOrientation() const;

    [[nodiscard]] inline ChannelHandle getHandle() const {
        return m_group.handle();
    }

    [[nodiscard]] const QString& getGroup() const {
        return m_group.name();
    }

    virtual ActiveState updateActiveState() = 0;
    [[nodiscard]] virtual bool isActive() const {
        return m_active;
    }

    void setPfl(bool enabled);
    [[nodiscard]] virtual bool isPflEnabled() const;
    void setMaster(bool enabled);
    [[nodiscard]] virtual bool isMasterEnabled() const;
    void setTalkover(bool enabled);
    [[nodiscard]] virtual bool isTalkoverEnabled() const;
    [[nodiscard]] inline bool isTalkoverChannel() const { return m_bIsTalkoverChannel; };
    [[nodiscard]] inline bool isPrimaryDeck() const {
        return m_bIsPrimaryDeck;
    };
    [[nodiscard]] int getChannelIndex() const {
        return m_channelIndex;
    }
    void setChannelIndex(const int channelIndex) {
        m_channelIndex = channelIndex;
    }

    virtual void postProcess(int iBuffersize) = 0;

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer() {
        return nullptr;
    }

  protected:
    const ChannelHandleAndGroup m_group;
    EffectsManager* m_pEffectsManager;

    EngineVuMeter m_vuMeter;
    PollingControlProxy m_sampleRate;
    const CSAMPLE* volatile m_sampleBuffer;

    // If set to true, this engine channel represents one of the primary playback decks.
    // It is used to check for valid bpm targets by the sync code.
    const bool m_bIsPrimaryDeck;
    bool m_active;

  private slots:
    void slotOrientationLeft(double v);
    void slotOrientationRight(double v);
    void slotOrientationCenter(double v);

  private:
    ControlPushButton m_master;
    ControlPushButton m_PFL;
    ControlPushButton m_orientation;
    ControlPushButton m_orientationLeft;
    ControlPushButton m_orientationRight;
    ControlPushButton m_orientationCenter;
    ControlPushButton m_talkover;
    bool m_bIsTalkoverChannel;
    int m_channelIndex;
};

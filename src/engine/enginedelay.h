#pragma once

#include "engine/engineobject.h"
#include "preferences/usersettings.h"

class ControlPotmeter;
class ControlProxy;

class EngineDelay : public EngineObject {
    Q_OBJECT
  public:
    EngineDelay(const QString& group, const ConfigKey& delayControl, bool bPersist = true);
    ~EngineDelay() override;

    void process(CSAMPLE* pInOut, int iBufferSize) override;

    void setDelay(double newDelay);

  public slots:
    void slotDelayChanged();

  private:
    ControlPotmeter* m_pDelayPot;
    ControlProxy* m_pSampleRate;
    CSAMPLE* m_pDelayBuffer;
    int m_iDelayPos{0};
    int m_iDelay{0};
};

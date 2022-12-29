#pragma once

#include "control/controlpotmeter.h"
#include "control/pollingcontrolproxy.h"
#include "engine/engineobject.h"

class ControlPotmeter;

class EngineVuMeter : public EngineObject {
    Q_OBJECT
  public:
    EngineVuMeter(const QString& group);
    ~EngineVuMeter() override = default;

    virtual void process(CSAMPLE* pInOut, const int iBufferSize);

    void reset();

  private:
    void doSmooth(CSAMPLE &currentVolume, CSAMPLE newVolume);

    ControlPotmeter m_ctrlVuMeter;
    ControlPotmeter m_ctrlVuMeterL;
    ControlPotmeter m_ctrlVuMeterR;
    CSAMPLE m_fRMSvolumeL;
    CSAMPLE m_fRMSvolumeSumL;
    CSAMPLE m_fRMSvolumeR;
    CSAMPLE m_fRMSvolumeSumR;
    int m_iSamplesCalculated;

    ControlPotmeter m_ctrlPeakIndicator;
    ControlPotmeter m_ctrlPeakIndicatorL;
    ControlPotmeter m_ctrlPeakIndicatorR;
    int m_peakDurationL;
    int m_peakDurationR;

    PollingControlProxy m_sampleRate;
};

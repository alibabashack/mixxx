#pragma once

#include <QTime>

#include "vinylcontrol.h"

class SteadyPitch {
    public:
        SteadyPitch(double threshold, bool assumeSteady);
        void reset(double pitch, double time);
        double check(double pitch, double time);
        [[nodiscard]] bool directionChanged(double pitch) const;
        [[nodiscard]] bool resyncDetected(double new_time) const;
    private:
        const bool m_bAssumeSteady;
        double m_dSteadyPitch;
        double m_dSteadyPitchTime;
        double m_dLastSteadyDur;
        double m_dLastTime;
        double m_dPitchThreshold;
        int m_iPlayDirection;
};

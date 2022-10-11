#pragma once

#include "util/types.h"

// HACK until we have Control 2.0
constexpr double MIXXX_XFADER_ADDITIVE = 0.0;
constexpr double MIXXX_XFADER_CONSTPWR = 1.0;

class EngineXfader {
  public:
    static double getPowerCalibration(double transform);
    static void getXfadeGains(double xfadePosition,
            double transform,
            double powerCalibration,
            double curve,
            bool reverse,
            CSAMPLE_GAIN* gain1,
            CSAMPLE_GAIN* gain2);

    static constexpr const char* const kXfaderConfigKey = "[Mixer Profile]";
    static constexpr double kTransformDefault = 1.0;
    static constexpr double kTransformMax = 1000.0;
    static constexpr double kTransformMin = 0.6;
};

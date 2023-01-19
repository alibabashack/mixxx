#pragma once
#include "track/bpm.h"

class Clock {
  public:
    virtual ~Clock() = default;

    [[nodiscard]] virtual double getBeatDistance() const = 0;
    virtual void updateLeaderBeatDistance(double beatDistance) = 0;

    [[nodiscard]] virtual mixxx::Bpm getBpm() const = 0;
    virtual void updateLeaderBpm(mixxx::Bpm bpm) = 0;
};

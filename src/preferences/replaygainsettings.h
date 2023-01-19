#pragma once

#include "preferences/usersettings.h"
#include "track/track_decl.h"

class ReplayGainSettings {
  public:
    ReplayGainSettings(UserSettingsPointer pConfig);

    [[nodiscard]] int getInitialReplayGainBoost() const;
    void setInitialReplayGainBoost(int value);
    [[nodiscard]] int getInitialDefaultBoost() const;
    void setInitialDefaultBoost(int value);
    [[nodiscard]] bool getReplayGainEnabled() const;
    void setReplayGainEnabled(bool value);
    [[nodiscard]] bool getReplayGainAnalyzerEnabled() const;
    void setReplayGainAnalyzerEnabled(bool value);
    [[nodiscard]] int getReplayGainAnalyzerVersion() const;
    void setReplayGainAnalyzerVersion(int value);
    [[nodiscard]] bool getReplayGainReanalyze() const;
    void setReplayGainReanalyze(bool value);

    [[nodiscard]] bool isAnalyzerEnabled(int version) const;
    [[nodiscard]] bool isAnalyzerDisabled(int version, TrackPointer tio) const;

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;
};

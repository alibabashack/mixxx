#pragma once

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"
#include "preferences/broadcastsettings.h"

/// Storage of broadcast settings for the encoders.
class EncoderBroadcastSettings : public EncoderSettings {
  public:
    explicit EncoderBroadcastSettings(BroadcastProfilePtr profile);
    ~EncoderBroadcastSettings() override = default;

    // Returns the list of quality values that it supports, to assign them to the slider
    [[nodiscard]] QList<int> getQualityValues() const override;
    // Returns the current quality value
    [[nodiscard]] int getQuality() const override;
    [[nodiscard]] int getQualityIndex() const override;
    [[nodiscard]] ChannelMode getChannelMode() const override;
    [[nodiscard]] QString getFormat() const override;

  private:
    QList<int> m_qualList;
    BroadcastProfilePtr m_pProfile;
};

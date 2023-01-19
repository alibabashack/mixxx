#pragma once

#include "encoder/encoderrecordingsettings.h"
#include "encoder/encoder.h"
#include "recording/defs_recording.h"

/// Storage of settings for Vorbis encoder
class EncoderVorbisSettings : public EncoderRecordingSettings {
    public:
    EncoderVorbisSettings(UserSettingsPointer pConfig);
    ~EncoderVorbisSettings() override = default;

    // Indicates that it uses the quality slider section of the preferences
    [[nodiscard]] bool usesQualitySlider() const override {
        return true;
    }

    // Returns the list of quality values that it supports, to assign them to the slider
    [[nodiscard]] QList<int> getQualityValues() const override;
    // Sets the quality value by its index
    void setQualityByIndex(int qualityIndex) override;
    // Returns the current quality value
    [[nodiscard]] int getQuality() const override;
    [[nodiscard]] int getQualityIndex() const override;

    // Returns the format of this encoder settings.
    [[nodiscard]] QString getFormat() const override {
        return ENCODING_OGG;
    }

  private:
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
};

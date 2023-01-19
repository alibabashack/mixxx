#pragma once

#include <QList>

#include "encoder/encoder.h"
#include "encoder/encodersettings.h"

class EncoderFdkAacSettings : public EncoderRecordingSettings {
  public:
    EncoderFdkAacSettings(UserSettingsPointer pConfig, QString format);
    ~EncoderFdkAacSettings() override;

    // Indicates that it uses the quality slider section of the preferences
    [[nodiscard]] bool usesQualitySlider() const override {
        return true;
    }

    // Returns the list of quality values that it supports, to assign them to the slider
    [[nodiscard]] QList<int> getQualityValues() const override;

    void setQualityByIndex(int qualityIndex) override;

    // Returns the current quality value
    [[nodiscard]] int getQuality() const override;
    [[nodiscard]] int getQualityIndex() const override;

    // Returns the format of this encoder settings.
    [[nodiscard]] QString getFormat() const override {
        return m_format;
    }

  private:
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
    QString m_format;
};

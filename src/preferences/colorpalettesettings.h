#pragma once

#include "preferences/usersettings.h"
#include "util/color/colorpalette.h"

// Saves ColorPalettes to and loads ColorPalettes from the mixxx.cfg file
class ColorPaletteSettings {
  public:
    explicit ColorPaletteSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    [[nodiscard]] ColorPalette getHotcueColorPalette(const QString& name) const;
    [[nodiscard]] ColorPalette getHotcueColorPalette() const;
    void setHotcueColorPalette(const ColorPalette& colorPalette);

    [[nodiscard]] ColorPalette getTrackColorPalette(const QString& name) const;
    [[nodiscard]] ColorPalette getTrackColorPalette() const;
    void setTrackColorPalette(const ColorPalette& colorPalette);

    [[nodiscard]] ColorPalette getColorPalette(
            const QString& name,
            const ColorPalette& defaultPalette) const;
    void setColorPalette(const QString& name, const ColorPalette& colorPalette);
    void removePalette(const QString& name);
    [[nodiscard]] QSet<QString> getColorPaletteNames() const;

  private:
    UserSettingsPointer m_pConfig;
};

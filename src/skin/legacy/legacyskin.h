#pragma once

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QPixmap>
#include <QScreen>
#include <QString>

#include "skin/skin.h"

namespace mixxx {
namespace skin {
namespace legacy {

class LegacySkin : public mixxx::skin::Skin {
  public:
    LegacySkin() = default;
    LegacySkin(const QFileInfo& path);

    static SkinPointer fromDirectory(const QDir& dir);

    [[nodiscard]] mixxx::skin::SkinType type() const override {
        return mixxx::skin::SkinType::Legacy;
    };
    [[nodiscard]] bool isValid() const override;
    [[nodiscard]] QFileInfo path() const override;
    [[nodiscard]] QPixmap preview(const QString& schemeName) const override;

    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString description() const override;
    [[nodiscard]] QList<QString> colorschemes() const override;

    [[nodiscard]] bool fitsScreenSize(const QScreen& screen) const override;
    LaunchImage* loadLaunchImage(QWidget* pParent, UserSettingsPointer pConfig) const override;
    QWidget* loadSkin(QWidget* pParent,
            UserSettingsPointer pConfig,
            QSet<ControlObject*>* pSkinCreatedControls,
            mixxx::CoreServices* pCoreServices) const override;

  private:
    [[nodiscard]] QFileInfo skinFile() const;

    QFileInfo m_path;
};

} // namespace legacy
} // namespace skin
} // namespace mixxx

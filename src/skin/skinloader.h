#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"
#include "skin/skin.h"

namespace mixxx {
namespace skin {

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            mixxx::CoreServices* pCoreServices);

    LaunchImage* loadLaunchImage(QWidget* pParent) const;

    [[nodiscard]] SkinPointer getSkin(const QString& skinName) const;
    [[nodiscard]] SkinPointer getConfiguredSkin() const;
    [[nodiscard]] QString getDefaultSkinName() const;
    [[nodiscard]] QList<QDir> getSkinSearchPaths() const;
    [[nodiscard]] QList<SkinPointer> getSkins() const;

  private:
    [[nodiscard]] QString pickResizableSkin(const QString& oldSkin) const;
    [[nodiscard]] SkinPointer skinFromDirectory(const QDir& dir) const;

    UserSettingsPointer m_pConfig;
};

} // namespace skin
} // namespace mixxx

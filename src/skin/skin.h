#pragma once

#include <QFileInfo>
#include <QList>
#include <QPixmap>
#include <QScreen>
#include <QSet>
#include <QString>
#include <QWidget>
#include <memory>

#include "preferences/usersettings.h"

class ControlObject;
class LaunchImage;

namespace mixxx {

class CoreServices;

namespace skin {

enum class SkinType {
    Legacy,
    QML,
};

class Skin {
  public:
    virtual ~Skin() = default;

    [[nodiscard]] virtual SkinType type() const = 0;

    [[nodiscard]] virtual bool isValid() const = 0;
    [[nodiscard]] virtual QFileInfo path() const = 0;
    [[nodiscard]] virtual QPixmap preview(const QString& schemeName) const = 0;

    [[nodiscard]] virtual QString name() const = 0;
    [[nodiscard]] virtual QString description() const = 0;
    [[nodiscard]] virtual QList<QString> colorschemes() const = 0;

    [[nodiscard]] virtual bool fitsScreenSize(const QScreen& screen) const = 0;

    virtual LaunchImage* loadLaunchImage(QWidget* pParent, UserSettingsPointer pConfig) const = 0;
    virtual QWidget* loadSkin(QWidget* pParent,
            UserSettingsPointer pConfig,
            QSet<ControlObject*>* pSkinCreatedControls,
            mixxx::CoreServices* pCoreServices) const = 0;
};

typedef std::shared_ptr<Skin> SkinPointer;

} // namespace skin
} // namespace mixxx

#pragma once

#include <QObject>
#include <QString>

#include "mixer/playermanager.h"

class BasePlayer : public QObject {
    Q_OBJECT
  public:
    BasePlayer(PlayerManager* pParent, const QString& group);
    ~BasePlayer() override = default;

    [[nodiscard]] inline const QString& getGroup() const {
        return m_group;
    }

  protected:
    PlayerManager* m_pPlayerManager;

  private:
    const QString m_group;
};

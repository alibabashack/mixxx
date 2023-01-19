#pragma once

#include <QHash>
#include <QStringList>
#include <QObject>

class Tooltips : public QObject {
    Q_OBJECT
  public:
    Tooltips();
    ~Tooltips() override;
    [[nodiscard]] QString tooltipForId(const QString& id) const;

  private:
    void addStandardTooltips();
    [[nodiscard]] QString tooltipSeparator() const;
    QList<QString>& add(const QString& id);

    QHash<QString, QStringList> m_tooltips;
};

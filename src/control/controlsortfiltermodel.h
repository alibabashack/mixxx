#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtQml>
#else
#define QML_ELEMENT
#endif

#include "control/controlmodel.h"

class ControlSortFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(int sortColumn READ sortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending NOTIFY sortDescendingChanged)
    QML_ELEMENT
  public:
    ControlSortFilterModel(QObject* pParent = nullptr);
    ~ControlSortFilterModel() override;

    bool sortDescending() const;

    Q_INVOKABLE void sortByColumn(int sortColumn, bool sortDescending);

  signals:
    void sortColumnChanged(int sortColumn);
    void sortDescendingChanged(bool sortDescending);

  private:
    ControlModel* m_pModel;
};

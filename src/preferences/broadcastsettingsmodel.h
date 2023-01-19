#pragma once

#include <QAbstractTableModel>
#include <QAbstractItemDelegate>
#include <QMap>
#include <QVariant>

#include "preferences/broadcastprofile.h"

class BroadcastSettings;
typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

class BroadcastSettingsModel : public QAbstractTableModel {
  Q_OBJECT
  public:
    BroadcastSettingsModel();

    void resetFromSettings(BroadcastSettingsPointer pSettings);
    bool addProfileToModel(BroadcastProfilePtr profile);
    void deleteProfileFromModel(BroadcastProfilePtr profile);
    BroadcastProfilePtr getProfileByName(const QString& profileName);
    QList<BroadcastProfilePtr> profiles() {
        return m_profiles.values();
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
            int role = Qt::EditRole) override;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* parent);

  private slots:
    void onProfileNameChanged(const QString& oldName, const QString& newName);
    void onConnectionStatusChanged(int newStatus);

  private:
    static QString connectionStatusString(BroadcastProfilePtr profile);
    static QColor connectionStatusBgColor(BroadcastProfilePtr profile);

    QMap<QString, BroadcastProfilePtr> m_profiles;
};

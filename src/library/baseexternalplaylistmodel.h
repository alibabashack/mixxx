#pragma once

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QtSql>

#include "library/basesqltablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/librarytablemodel.h"
#include "library/trackmodel.h"

class BaseExternalPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalPlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager,
                              const char* settingsNamespace, const QString& playlistsTable,
                              const QString& playlistTracksTable, QSharedPointer<BaseTrackCache> trackSource);

    ~BaseExternalPlaylistModel() override;

    void setPlaylist(const QString& path_name);

    [[nodiscard]] TrackPointer getTrack(const QModelIndex& index) const override;
    [[nodiscard]] TrackId getTrackId(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] Capabilities getCapabilities() const override;
    [[nodiscard]] QString modelKey(bool noSearch) const override;

  private:
    [[nodiscard]] TrackId doGetTrackId(const TrackPointer& pTrack) const override;

    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QSharedPointer<BaseTrackCache> m_trackSource;
    int m_currentPlaylistId;
    QHash<int, QString> m_searchTexts;
};

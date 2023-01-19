#pragma once

#include <QModelIndex>
#include <QObject>
#include <QString>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"

class TrackCollection;

class BaseExternalTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalTrackModel(QObject* parent,
                           TrackCollectionManager* pTrackCollectionManager,
                           const char* settingsNamespace,
                           const QString& trackTable,
                           QSharedPointer<BaseTrackCache> trackSource);
    ~BaseExternalTrackModel() override;

    [[nodiscard]] Capabilities getCapabilities() const override;
    [[nodiscard]] TrackId getTrackId(const QModelIndex& index) const override;
    [[nodiscard]] TrackPointer getTrack(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  private:
    [[nodiscard]] TrackId doGetTrackId(const TrackPointer& pTrack) const override;
};

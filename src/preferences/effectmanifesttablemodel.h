#pragma once

#include <QAbstractItemDelegate>
#include <QAbstractTableModel>
#include <QMap>
#include <QVariant>

#include "effects/backends/effectmanifest.h"

/// EffectManifestTableModel represents a list of EffectManifests as a table
/// with a column for the BackendType and a column for the effect name.
/// EffectManifestTableModel is used by DlgPrefEffects to allow the user to drag
/// and drop between the lists of visible and hidden effects. It also allows the
/// user to rearrange the list of visible effects so they can choose which
/// effects can be loaded by controllers.
class EffectManifestTableModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    EffectManifestTableModel(QObject* parent, EffectsBackendManagerPointer pBackendManager);
    ~EffectManifestTableModel() override = default;

    [[nodiscard]] const QList<EffectManifestPointer>& getList() const {
        return m_manifests;
    }
    void setList(const QList<EffectManifestPointer>& newList);

    // These functions are required for displaying data.
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant headerData(int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // These functions are required for drag and drop.
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(
            const QMimeData* data,
            Qt::DropAction action,
            int row,
            int column,
            const QModelIndex& parent) override;
    [[nodiscard]] QStringList mimeTypes() const override;
    [[nodiscard]] Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  private:
    QList<EffectManifestPointer> m_manifests;
    EffectsBackendManagerPointer m_pBackendManager;
};

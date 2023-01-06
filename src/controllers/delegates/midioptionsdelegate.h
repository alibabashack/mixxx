#pragma once

#include <QStyledItemDelegate>
#include <QTableView>

#include "controllers/midi/midimessage.h"

class MidiOptionsDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    MidiOptionsDelegate(QObject* pParent);
    ~MidiOptionsDelegate() override;

    [[nodiscard]] QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};

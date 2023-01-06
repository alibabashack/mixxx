#pragma once

#include <QStyledItemDelegate>

class MidiByteDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    MidiByteDelegate(QObject* pParent);
    ~MidiByteDelegate() override;

    [[nodiscard]] QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};

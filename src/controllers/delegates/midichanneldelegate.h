#pragma once

#include <QStyledItemDelegate>

class MidiChannelDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    MidiChannelDelegate(QObject* pParent);
    ~MidiChannelDelegate() override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    QString displayText(const QVariant& value, const QLocale& locale) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};

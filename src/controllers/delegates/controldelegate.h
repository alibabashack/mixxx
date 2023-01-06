#pragma once

#include <QStyledItemDelegate>

#include "controllers/controlpickermenu.h"

class ControlDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    ControlDelegate(QObject* pParent);
    ~ControlDelegate() override;

    inline void setMidiOptionsColumn(int column) {
        m_iMidiOptionsColumn = column;
    }

    [[nodiscard]] QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

  private:
    ControlPickerMenu* m_pPicker;
    int m_iMidiOptionsColumn;
    // HACK(rryan): Does the last painted index have a script
    // MidiOption. displayText does not give us the current QModelIndex so we
    // can't check there.
    mutable bool m_bIsIndexScript;
};

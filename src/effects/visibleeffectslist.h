#pragma once

#include <QList>

#include "effects/backends/effectmanifest.h"

QT_FORWARD_DECLARE_CLASS(QDomDocument);

class VisibleEffectsList : public QObject {
    Q_OBJECT

  public:
    [[nodiscard]] const QList<EffectManifestPointer>& getList() const {
        return m_list;
    }

    [[nodiscard]] int indexOf(EffectManifestPointer pManifest) const;
    [[nodiscard]] const EffectManifestPointer at(int index) const;
    [[nodiscard]] const EffectManifestPointer next(const EffectManifestPointer pManifest) const;
    [[nodiscard]] const EffectManifestPointer previous(const EffectManifestPointer pManifest) const;

    void setList(const QList<EffectManifestPointer>& newList);
    void readEffectsXml(const QDomDocument& doc, EffectsBackendManagerPointer pBackendManager);
    void saveEffectsXml(QDomDocument* pDoc);

  signals:
    void visibleEffectsListChanged();

  private:
    QList<EffectManifestPointer> m_list;
};

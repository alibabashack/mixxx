#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include "control/controlobject.h"
#include "effects/effectparameterslotbase.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;

/// Refer to EffectParameterSlotBase for documentation
class EffectButtonParameterSlot : public EffectParameterSlotBase {
    Q_OBJECT
  public:
    EffectButtonParameterSlot(const QString& group, const unsigned int iParameterSlotNumber);
    ~EffectButtonParameterSlot() override;

    static QString formatItemPrefix(const unsigned int iParameterSlotNumber) {
        return QString("button_parameter%1").arg(iParameterSlotNumber + 1);
    }

    void loadParameter(EffectParameterPointer pEffectParameter) override;

    void clear() override;

    void setParameter(double value) override;

  private:
    [[nodiscard]] QString debugString() const {
        return QString("EffectButtonParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterSlotNumber);
    }

    // Control exposed to the rest of Mixxx
    ControlPushButton* m_pControlValue;

    DISALLOW_COPY_AND_ASSIGN(EffectButtonParameterSlot);
};

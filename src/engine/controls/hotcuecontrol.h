#pragma once

#include <QObject>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "preferences/usersettings.h"
#include "track/cue.h"

/// Used for requesting a specific hotcue type when activating/setting a
/// hotcue. Auto will make CueControl determine the type automatically (i.e.
/// create a loop cue if a loop is set, and a regular cue in all other cases).
enum class HotcueSetMode {
    Auto,
    Cue,
    Loop,
};

/// A `HotcueControl` represents a hotcue slot. It can either be empty or have
/// a (hot-)cue attached to it.
class HotcueControl : public QObject {
    Q_OBJECT
  public:
    /// Describes the current status of the hotcue
    enum class Status {
        /// Hotuce not set
        Empty = 0,
        /// Hotcue is set and can be used
        Set = 1,
        /// Hotcue is currently active (this only applies to Saved Loop cues
        /// while their loop is enabled). This status can be used by skins or
        /// controller mappings to highlight the cue control that has saved the current loop,
        /// because resizing or moving the loop will make persistent changes to
        /// the cue.
        Active = 2,
    };

    HotcueControl(const QString& group, int hotcueIndex);
    ~HotcueControl() override;

    int getHotcueIndex() const {
        return m_hotcueIndex;
    }

    CuePointer getCue() const {
        return m_pCue;
    }
    void setCue(const CuePointer& pCue);
    void resetCue();

    mixxx::audio::FramePos getPosition() const;
    void setPosition(mixxx::audio::FramePos position);

    mixxx::audio::FramePos getEndPosition() const;
    void setEndPosition(mixxx::audio::FramePos endPosition);

    void setType(mixxx::CueType type);

    void setStatus(HotcueControl::Status status);
    HotcueControl::Status getStatus() const;

    void setColor(mixxx::RgbColor::optional_t newColor);
    mixxx::RgbColor::optional_t getColor() const;

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    mixxx::CueType getPreviewingType() const {
        return m_previewingType.getValue();
    }

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    mixxx::audio::FramePos getPreviewingPosition() const {
        return m_previewingPosition.getValue();
    }

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    void cachePreviewingStartState() {
        if (m_pCue) {
            m_previewingPosition.setValue(m_pCue->getPosition());
            m_previewingType.setValue(m_pCue->getType());
        } else {
            m_previewingType.setValue(mixxx::CueType::Invalid);
        }
    }

  private slots:
    void slotHotcueSet(double v);
    void slotHotcueSetCue(double v);
    void slotHotcueSetLoop(double v);
    void slotHotcueGoto(double v);
    void slotHotcueGotoAndPlay(double v);
    void slotHotcueGotoAndStop(double v);
    void slotHotcueGotoAndLoop(double v);
    void slotHotcueCueLoop(double v);
    void slotHotcueActivate(double v);
    void slotHotcueActivateCue(double v);
    void slotHotcueActivateLoop(double v);
    void slotHotcueActivatePreview(double v);
    void slotHotcueClear(double v);
    void slotHotcueEndPositionChanged(double newPosition);
    void slotHotcuePositionChanged(double newPosition);
    void slotHotcueColorChangeRequest(double newColor);
    void slotHotcueColorChanged(double newColor);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v, HotcueSetMode mode);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndPlay(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueGotoAndLoop(HotcueControl* pHotcue, double v);
    void hotcueCueLoop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v, HotcueSetMode mode);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pHotcue, double newEndPosition);
    void hotcueColorChanged(HotcueControl* pHotcue, double newColor);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(const QString& name);

    const QString m_group;
    const int m_hotcueIndex;
    CuePointer m_pCue;

    // Hotcue state controls
    std::unique_ptr<ControlObject> m_hotcuePosition;
    std::unique_ptr<ControlObject> m_hotcueEndPosition;
    std::unique_ptr<ControlObject> m_pHotcueStatus;
    std::unique_ptr<ControlObject> m_hotcueType;
    std::unique_ptr<ControlObject> m_hotcueColor;
    // Hotcue button controls
    std::unique_ptr<ControlPushButton> m_hotcueSet;
    std::unique_ptr<ControlPushButton> m_hotcueSetCue;
    std::unique_ptr<ControlPushButton> m_hotcueSetLoop;
    std::unique_ptr<ControlPushButton> m_hotcueGoto;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndPlay;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndStop;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndLoop;
    std::unique_ptr<ControlPushButton> m_hotcueCueLoop;
    std::unique_ptr<ControlPushButton> m_hotcueActivate;
    std::unique_ptr<ControlPushButton> m_hotcueActivateCue;
    std::unique_ptr<ControlPushButton> m_hotcueActivateLoop;
    std::unique_ptr<ControlPushButton> m_hotcueActivatePreview;
    std::unique_ptr<ControlPushButton> m_hotcueClear;

    ControlValueAtomic<mixxx::CueType> m_previewingType;
    ControlValueAtomic<mixxx::audio::FramePos> m_previewingPosition;
};

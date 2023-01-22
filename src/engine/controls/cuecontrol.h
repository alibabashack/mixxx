#pragma once

#include <gtest/gtest_prod.h>

#include <QAtomicInt>
#include <QAtomicPointer>
#include <QList>

#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "engine/controls/hotcuecontrol.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/usersettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/compatibility/qmutex.h"
#include "util/parented_ptr.h"

constexpr int kNumHotCues = 37;

class ControlObject;
class ControlPushButton;
class ControlIndicator;

enum class CueMode {
    Mixxx,
    Pioneer,
    Denon,
    Numark,
    MixxxNoBlinking,
    CueAndPlay
};

enum class SeekOnLoadMode {
    MainCue = 0,    // Use main cue point
    Beginning = 1,  // Use 0:00.000
    FirstSound = 2, // Skip leading silence
    IntroStart = 3, // Use intro start cue point
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const QString& group,
            UserSettingsPointer pConfig);
    ~CueControl() override;

    void hintReader(gsl::not_null<HintVector*> pHintList) override;
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay, bool playPossible);
    void updateIndicators();
    bool isTrackAtIntroCue();
    void resetIndicators();
    SeekOnLoadMode getSeekOnLoadPreference();
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  signals:
    void loopRemove();

  public slots:
    void slotLoopReset();
    void slotLoopEnabledChanged(bool enabled);
    void slotLoopUpdated(mixxx::audio::FramePos startPosition, mixxx::audio::FramePos endPosition);

  private slots:
    void quantizeChanged(double v);

    void cueUpdated();
    void trackAnalyzed();
    void trackCuesUpdated();
    void hotcueSet(HotcueControl* pControl, double v, HotcueSetMode mode);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndPlay(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueGotoAndLoop(HotcueControl* pControl, double v);
    void hotcueCueLoop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v, HotcueSetMode mode);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void updateCurrentlyPreviewingIndex(int hotcueIndex);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pControl, double newEndPosition);

    void hotcueFocusColorNext(double v);
    void hotcueFocusColorPrev(double v);

    void passthroughChanged(double v);

    void cueSet(double v);
    void cueClear(double v);
    void cueGoto(double v);
    void cueGotoAndPlay(double v);
    void cueGotoAndStop(double v);
    void cuePreview(double v);
    FRIEND_TEST(CueControlTest, SeekOnSetCueCDJ);
    void cueCDJ(double v);
    void cueDenon(double v);
    FRIEND_TEST(CueControlTest, SeekOnSetCuePlay);
    void cuePlay(double v);
    void cueDefault(double v);
    void pause(double v);
    void playStutter(double v);

    void introStartSet(double v);
    void introStartClear(double v);
    void introStartActivate(double v);
    void introEndSet(double v);
    void introEndClear(double v);
    void introEndActivate(double v);
    void outroStartSet(double v);
    void outroStartClear(double v);
    void outroStartActivate(double v);
    void outroEndSet(double v);
    void outroEndClear(double v);
    void outroEndActivate(double v);

  private:
    enum class TrackAt {
        Cue,
        End,
        ElseWhere
    };

    // These methods are not thread safe, only call them when the lock is held.
    void createControls();
    void connectControls();
    void disconnectControls();

    void attachCue(const CuePointer& pCue, HotcueControl* pControl);
    void detachCue(HotcueControl* pControl);
    void setCurrentSavedLoopControlAndActivate(HotcueControl* pControl);
    void loadCuesFromTrack();
    mixxx::audio::FramePos quantizeCuePoint(mixxx::audio::FramePos position);
    mixxx::audio::FramePos getQuantizedCurrentPosition();
    TrackAt getTrackAt() const;
    void seekOnLoad(mixxx::audio::FramePos seekOnLoadPosition);
    void setHotcueFocusIndex(int hotcueIndex);
    int getHotcueFocusIndex() const;

    UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    QAtomicInt m_currentlyPreviewingIndex;
    ControlObject* m_pPlay;
    ControlObject* m_pStopButton;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pClosestBeat;
    parented_ptr<ControlProxy> m_pLoopStartPosition;
    parented_ptr<ControlProxy> m_pLoopEndPosition;
    parented_ptr<ControlProxy> m_pLoopEnabled;
    parented_ptr<ControlProxy> m_pBeatLoopActivate;
    parented_ptr<ControlProxy> m_pBeatLoopSize;
    bool m_bypassCueSetByPlay;
    ControlValueAtomic<mixxx::audio::FramePos> m_usedSeekOnLoadPosition;

    const int m_iNumHotCues;
    QList<HotcueControl*> m_hotcueControls;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pCuePoint;
    ControlObject* m_pCueMode;
    ControlPushButton* m_pCueSet;
    ControlPushButton* m_pCueClear;
    ControlPushButton* m_pCueCDJ;
    ControlPushButton* m_pCueDefault;
    ControlPushButton* m_pPlayStutter;
    ControlIndicator* m_pCueIndicator;
    ControlIndicator* m_pPlayIndicator;
    ControlObject* m_pPlayLatched;
    ControlPushButton* m_pCueGoto;
    ControlPushButton* m_pCueGotoAndPlay;
    ControlPushButton* m_pCuePlay;
    ControlPushButton* m_pCueGotoAndStop;
    ControlPushButton* m_pCuePreview;

    ControlObject* m_pIntroStartPosition;
    ControlObject* m_pIntroStartEnabled;
    ControlPushButton* m_pIntroStartSet;
    ControlPushButton* m_pIntroStartClear;
    ControlPushButton* m_pIntroStartActivate;

    ControlObject* m_pIntroEndPosition;
    ControlObject* m_pIntroEndEnabled;
    ControlPushButton* m_pIntroEndSet;
    ControlPushButton* m_pIntroEndClear;
    ControlPushButton* m_pIntroEndActivate;

    ControlObject* m_pOutroStartPosition;
    ControlObject* m_pOutroStartEnabled;
    ControlPushButton* m_pOutroStartSet;
    ControlPushButton* m_pOutroStartClear;
    ControlPushButton* m_pOutroStartActivate;

    ControlObject* m_pOutroEndPosition;
    ControlObject* m_pOutroEndEnabled;
    ControlPushButton* m_pOutroEndSet;
    ControlPushButton* m_pOutroEndClear;
    ControlPushButton* m_pOutroEndActivate;

    ControlProxy* m_pVinylControlEnabled;
    ControlProxy* m_pVinylControlMode;

    ControlObject* m_pHotcueFocus;
    ControlObject* m_pHotcueFocusColorNext;
    ControlObject* m_pHotcueFocusColorPrev;

    parented_ptr<ControlProxy> m_pPassthrough;

    QAtomicPointer<HotcueControl> m_pCurrentSavedLoopControl;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    // Must be locked when using the m_pLoadedTrack and it's properties
    QT_RECURSIVE_MUTEX m_trackMutex;
    TrackPointer m_pLoadedTrack; // is written from an engine worker thread

    friend class HotcueControlTest;
};

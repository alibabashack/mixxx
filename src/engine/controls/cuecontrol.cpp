#include "engine/controls/cuecontrol.h"

#include "control/controlindicator.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/hotcue.h"
#include "engine/enginebuffer.h"
#include "moc_cuecontrol.cpp"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/sample.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {

// TODO: Convert these doubles to a standard enum
// and convert elseif logic to switch statements
constexpr double CUE_MODE_MIXXX = 0.0;
constexpr double CUE_MODE_PIONEER = 1.0;
constexpr double CUE_MODE_DENON = 2.0;
constexpr double CUE_MODE_NUMARK = 3.0;
constexpr double CUE_MODE_MIXXX_NO_BLINK = 4.0;
constexpr double CUE_MODE_CUP = 5.0;

/// Used for a common tracking of the previewing Hotcue in m_currentlyPreviewingIndex
constexpr int kMainCueIndex = kNumHotCues;

void appendCueHint(gsl::not_null<HintVector*> pHintList,
        const mixxx::audio::FramePos& frame,
        Hint::Type type) {
    if (frame.isValid()) {
        const Hint cueHint = {
                /*.frame =*/static_cast<SINT>(frame.toLowerFrameBoundary().value()),
                /*.frameCount =*/Hint::kFrameCountForward,
                /*.type =*/type};
        pHintList->append(cueHint);
    }
}

void appendCueHint(gsl::not_null<HintVector*> pHintList, const double playPos, Hint::Type type) {
    const auto frame = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(playPos);
    appendCueHint(pHintList, frame, type);
}

std::array<std::unique_ptr<HotcueControl>, kNumHotCues> buildHotcueControls(const QString& group) {
    std::array<std::unique_ptr<HotcueControl>, kNumHotCues> controls;
    for (size_t i = 0; i < controls.size(); i++) {
        controls[i] = std::make_unique<HotcueControl>(group, i);
    }
    return controls;
}

} // namespace

CueControl::CueControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_currentlyPreviewingIndex(Cue::kNoHotCue),
          m_pPlay(ControlObject::getControl(ConfigKey(group, "play"))),
          m_pStopButton(ControlObject::getControl(ConfigKey(group, "stop"))),
          m_pQuantizeEnabled(ControlObject::getControl(ConfigKey(group, "quantize"))),
          m_pClosestBeat(ControlObject::getControl(ConfigKey(group, "beat_closest"))),
          m_loopStartPosition(group, "loop_start_position", this),
          m_loopEndPosition(group, "loop_end_position", this),
          m_loopEnabled(group, "loop_enabled", this),
          m_beatLoopActivate(group, "beatloop_activate", this),
          m_beatLoopSize(group, "beatloop_size", this),
          m_bypassCueSetByPlay(false),
          m_hotcueControls(buildHotcueControls(group)),
          m_pTrackSamples(ControlObject::getControl(ConfigKey(group, "track_samples"))),
          m_cuePoint(ConfigKey(group, "cue_point")),
          m_cueMode(ConfigKey(group, "cue_mode")),
          m_cueSet(ConfigKey(m_group, "cue_set")),
          m_cueClear(ConfigKey(m_group, "cue_clear")),
          m_cueCDJ(ConfigKey(m_group, "cue_cdj")),
          m_cueDefault(ConfigKey(m_group, "cue_default")),
          m_playStutter(ConfigKey(m_group, "play_stutter")),
          m_pueIndicator(ConfigKey(m_group, "cue_indicator")),
          m_playIndicator(ConfigKey(m_group, "play_indicator")),
          m_playLatched(ConfigKey(m_group, "play_latched")),
          m_cueGoto(ConfigKey(m_group, "cue_goto")),
          m_cueGotoAndPlay(ConfigKey(m_group, "cue_gotoandplay")),
          m_cuePlay(ConfigKey(m_group, "cue_play")),
          m_cueGotoAndStop(ConfigKey(m_group, "cue_gotoandstop")),
          m_cuePreview(ConfigKey(m_group, "cue_preview")),
          m_introStartPosition(ConfigKey(m_group, "intro_start_position")),
          m_introStartEnabled(ConfigKey(m_group, "intro_start_enabled")),
          m_introStartSet(ConfigKey(m_group, "intro_start_set")),
          m_introStartClear(ConfigKey(m_group, "intro_start_clear")),
          m_introStartActivate(ConfigKey(m_group, "intro_start_activate")),
          m_introEndPosition(ConfigKey(m_group, "intro_end_position")),
          m_introEndEnabled(ConfigKey(m_group, "intro_end_enabled")),
          m_introEndSet(ConfigKey(m_group, "intro_end_set")),
          m_introEndClear(ConfigKey(m_group, "intro_end_clear")),
          m_introEndActivate(ConfigKey(m_group, "intro_end_activate")),
          m_outroStartPosition(ConfigKey(m_group, "outro_start_position")),
          m_outroStartEnabled(ConfigKey(m_group, "outro_start_enabled")),
          m_outroStartSet(ConfigKey(m_group, "outro_start_set")),
          m_outroStartClear(ConfigKey(m_group, "outro_start_clear")),
          m_outroStartActivate(ConfigKey(m_group, "outro_start_activate")),
          m_outroEndPosition(ConfigKey(m_group, "outro_end_position")),
          m_outroEndEnabled(ConfigKey(m_group, "outro_end_enabled")),
          m_outroEndSet(ConfigKey(m_group, "outro_end_set")),
          m_outroEndClear(ConfigKey(m_group, "outro_end_clear")),
          m_outroEndActivate(ConfigKey(m_group, "outro_end_activate")),
          m_vinylControlEnabled(m_group, "vinylcontrol_enabled"),
          m_vinylControlMode(m_group, "vinylcontrol_mode"),
          m_hotcueFocus(ConfigKey(m_group, "hotcue_focus")),
          m_hotcueFocusColorNext(ConfigKey(m_group, "hotcue_focus_color_next")),
          m_hotcueFocusColorPrev(ConfigKey(m_group, "hotcue_focus_color_prev")),
          m_pPassthrough(make_parented<ControlProxy>(group, "passthrough", this)),
          m_pCurrentSavedLoopControl(nullptr),
          m_trackMutex(QT_RECURSIVE_MUTEX_INIT) {
    // To silence a compiler warning about CUE_MODE_PIONEER.
    Q_UNUSED(CUE_MODE_PIONEER);

    createControls();
    connectControls();

    connect(m_pQuantizeEnabled, &ControlObject::valueChanged,
            this, &CueControl::quantizeChanged,
            Qt::DirectConnection);

    m_cuePoint.set(Cue::kNoPosition);

    m_pPassthrough->connectValueChanged(this,
            &CueControl::passthroughChanged,
            Qt::DirectConnection);
}

void CueControl::createControls() {
    m_cueSet.setButtonMode(ControlPushButton::TRIGGER);
    m_cueClear.setButtonMode(ControlPushButton::TRIGGER);
    m_playLatched.setReadOnly();
    m_introStartPosition.set(Cue::kNoPosition);
    m_introStartEnabled.setReadOnly();
    m_introEndPosition.set(Cue::kNoPosition);
    m_introEndEnabled.setReadOnly();
    m_outroStartPosition.set(Cue::kNoPosition);
    m_outroStartEnabled.setReadOnly();
    m_outroEndPosition.set(Cue::kNoPosition);
    m_outroEndEnabled.setReadOnly();
    setHotcueFocusIndex(Cue::kNoHotCue);
}

void CueControl::connectControls() {
    // Main Cue controls
    connect(&m_cueSet,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueSet,
            Qt::DirectConnection);
    connect(&m_cueClear,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueClear,
            Qt::DirectConnection);
    connect(&m_cueGoto,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGoto,
            Qt::DirectConnection);
    connect(&m_cueGotoAndPlay,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGotoAndPlay,
            Qt::DirectConnection);
    connect(&m_cuePlay,
            &ControlObject::valueChanged,
            this,
            &CueControl::cuePlay,
            Qt::DirectConnection);
    connect(&m_cueGotoAndStop,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGotoAndStop,
            Qt::DirectConnection);
    connect(&m_cuePreview,
            &ControlObject::valueChanged,
            this,
            &CueControl::cuePreview,
            Qt::DirectConnection);
    connect(&m_cueCDJ,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueCDJ,
            Qt::DirectConnection);
    connect(&m_cueDefault,
            &ControlObject::valueChanged,
            this,
            &CueControl::cueDefault,
            Qt::DirectConnection);
    connect(&m_playStutter,
            &ControlObject::valueChanged,
            this,
            &CueControl::playStutter,
            Qt::DirectConnection);

    connect(&m_introStartSet,
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartSet,
            Qt::DirectConnection);
    connect(&m_introStartClear,
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartClear,
            Qt::DirectConnection);
    connect(&m_introStartActivate,
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartActivate,
            Qt::DirectConnection);
    connect(&m_introEndSet,
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndSet,
            Qt::DirectConnection);
    connect(&m_introEndClear,
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndClear,
            Qt::DirectConnection);
    connect(&m_introEndActivate,
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndActivate,
            Qt::DirectConnection);

    connect(&m_outroStartSet,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartSet,
            Qt::DirectConnection);
    connect(&m_outroStartClear,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartClear,
            Qt::DirectConnection);
    connect(&m_outroStartActivate,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartActivate,
            Qt::DirectConnection);
    connect(&m_outroEndSet,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndSet,
            Qt::DirectConnection);
    connect(&m_outroEndClear,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndClear,
            Qt::DirectConnection);
    connect(&m_outroEndActivate,
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndActivate,
            Qt::DirectConnection);

    connect(&m_hotcueFocusColorPrev,
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorPrev,
            Qt::DirectConnection);
    connect(&m_hotcueFocusColorNext,
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorNext,
            Qt::DirectConnection);

    // Hotcue controls
    for (const auto& pControl : qAsConst(m_hotcueControls)) {
        connect(pControl.get(), &HotcueControl::hotcuePositionChanged,
                this, &CueControl::hotcuePositionChanged,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueEndPositionChanged,
                this,
                &CueControl::hotcueEndPositionChanged,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueSet,
                this,
                &CueControl::hotcueSet,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueGoto,
                this,
                &CueControl::hotcueGoto,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueGotoAndPlay,
                this,
                &CueControl::hotcueGotoAndPlay,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueGotoAndStop,
                this,
                &CueControl::hotcueGotoAndStop,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueGotoAndLoop,
                this,
                &CueControl::hotcueGotoAndLoop,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueCueLoop,
                this,
                &CueControl::hotcueCueLoop,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueActivate,
                this,
                &CueControl::hotcueActivate,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueActivatePreview,
                this,
                &CueControl::hotcueActivatePreview,
                Qt::DirectConnection);
        connect(pControl.get(),
                &HotcueControl::hotcueClear,
                this,
                &CueControl::hotcueClear,
                Qt::DirectConnection);
    }
}

void CueControl::disconnectControls() {
    disconnect(&m_cueSet, nullptr, this, nullptr);
    disconnect(&m_cueClear, nullptr, this, nullptr);
    disconnect(&m_cueGoto, nullptr, this, nullptr);
    disconnect(&m_cueGotoAndPlay, nullptr, this, nullptr);
    disconnect(&m_cuePlay, nullptr, this, nullptr);
    disconnect(&m_cueGotoAndStop, nullptr, this, nullptr);
    disconnect(&m_cuePreview, nullptr, this, nullptr);
    disconnect(&m_cueCDJ, nullptr, this, nullptr);
    disconnect(&m_cueDefault, nullptr, this, nullptr);
    disconnect(&m_playStutter, nullptr, this, nullptr);

    disconnect(&m_introStartSet, nullptr, this, nullptr);
    disconnect(&m_introStartClear, nullptr, this, nullptr);
    disconnect(&m_introStartActivate, nullptr, this, nullptr);
    disconnect(&m_introEndSet, nullptr, this, nullptr);
    disconnect(&m_introEndClear, nullptr, this, nullptr);
    disconnect(&m_introEndActivate, nullptr, this, nullptr);

    disconnect(&m_outroStartSet, nullptr, this, nullptr);
    disconnect(&m_outroStartClear, nullptr, this, nullptr);
    disconnect(&m_outroStartActivate, nullptr, this, nullptr);
    disconnect(&m_outroEndSet, nullptr, this, nullptr);
    disconnect(&m_outroEndClear, nullptr, this, nullptr);
    disconnect(&m_outroEndActivate, nullptr, this, nullptr);

    disconnect(&m_hotcueFocusColorPrev, nullptr, this, nullptr);
    disconnect(&m_hotcueFocusColorNext, nullptr, this, nullptr);

    for (const auto& pControl : qAsConst(m_hotcueControls)) {
        disconnect(pControl.get(), nullptr, this, nullptr);
    }
}

void CueControl::passthroughChanged(double enabled) {
    if (enabled > 0) {
        // If passthrough was enabled seeking and playing is prohibited, and the
        // waveform and overview are blocked.
        // Disconnect all cue controls to prevent cue changes without UI feedback.
        disconnectControls();
    } else {
        // Reconnect all controls when deck returns to regular mode.
        connectControls();
    }
}

void CueControl::attachCue(const CuePointer& pCue, HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }
    detachCue(pControl);
    connect(pCue.get(),
            &Cue::updated,
            this,
            &CueControl::cueUpdated,
            Qt::DirectConnection);

    pControl->setCue(pCue);
}

void CueControl::detachCue(HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    disconnect(pCue.get(), nullptr, this, nullptr);
    m_pCurrentSavedLoopControl.testAndSetRelease(pControl, nullptr);
    pControl->resetCue();
}

// This is called from the EngineWokerThread and ends with the initial seek
// via seekOnLoad(). There is the theoretical and pending issue of a delayed control
// command intended for the old track that might be performed instead.
void CueControl::trackLoaded(TrackPointer pNewTrack) {
    auto lock = lockMutex(&m_trackMutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), nullptr, this, nullptr);

        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);

        for (const auto& pControl : qAsConst(m_hotcueControls)) {
            detachCue(pControl.get());
        }

        m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
        m_cuePoint.set(Cue::kNoPosition);
        m_introStartPosition.set(Cue::kNoPosition);
        m_introStartEnabled.forceSet(0.0);
        m_introEndPosition.set(Cue::kNoPosition);
        m_introEndEnabled.forceSet(0.0);
        m_outroStartPosition.set(Cue::kNoPosition);
        m_outroStartEnabled.forceSet(0.0);
        m_outroEndPosition.set(Cue::kNoPosition);
        m_outroEndEnabled.forceSet(0.0);
        setHotcueFocusIndex(Cue::kNoHotCue);
        m_pLoadedTrack.reset();
        m_usedSeekOnLoadPosition.setValue(mixxx::audio::kStartFramePos);
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(),
            &Track::analyzed,
            this,
            &CueControl::trackAnalyzed,
            Qt::DirectConnection);

    connect(m_pLoadedTrack.get(),
            &Track::cuesUpdated,
            this,
            &CueControl::trackCuesUpdated,
            Qt::DirectConnection);

    connect(m_pLoadedTrack.get(),
            &Track::loopRemove,
            this,
            &CueControl::loopRemove);

    lock.unlock();

    // Use pNewTrack from now, because m_pLoadedTrack might have been reset
    // immediately after leaving the locking scope!

    // Update COs with cues from track.
    loadCuesFromTrack();

    // Seek track according to SeekOnLoadMode.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadPreference();

    switch (seekOnLoadMode) {
    case SeekOnLoadMode::Beginning:
        // This allows users to load tracks and have the needle-drop be maintained.
        if (!(m_vinylControlEnabled.toBool() &&
                    m_vinylControlMode.get() == MIXXX_VCMODE_ABSOLUTE)) {
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        break;
    case SeekOnLoadMode::FirstSound: {
        CuePointer pN60dBSound =
                pNewTrack->findCueByType(mixxx::CueType::N60dBSound);
        mixxx::audio::FramePos n60dBSoundPosition;
        if (pN60dBSound) {
            n60dBSoundPosition = pN60dBSound->getPosition();
        }
        if (n60dBSoundPosition.isValid()) {
            seekOnLoad(n60dBSoundPosition);
        } else {
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        break;
    }
    case SeekOnLoadMode::MainCue: {
        // Take main cue position from CO instead of cue point list because
        // value in CO will be quantized if quantization is enabled
        // while value in cue point list will never be quantized.
        // This prevents jumps when track analysis finishes while quantization is enabled.
        const auto mainCuePosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_cuePoint.get());
        if (mainCuePosition.isValid()) {
            seekOnLoad(mainCuePosition);
        } else {
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        break;
    }
    case SeekOnLoadMode::IntroStart: {
        const auto introStartPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_introStartPosition.get());
        if (introStartPosition.isValid()) {
            seekOnLoad(introStartPosition);
        } else {
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        break;
    }
    default:
        DEBUG_ASSERT(!"Unknown enum value");
        seekOnLoad(mixxx::audio::kStartFramePos);
        break;
    }
}

void CueControl::seekOnLoad(mixxx::audio::FramePos seekOnLoadPosition) {
    DEBUG_ASSERT(seekOnLoadPosition.isValid());
    seekExact(seekOnLoadPosition);
    m_usedSeekOnLoadPosition.setValue(seekOnLoadPosition);
}

void CueControl::cueUpdated() {
    //auto lock = lockMutex(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::loadCuesFromTrack() {
    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    QSet<int> active_hotcues;
    CuePointer pMainCue;
    CuePointer pIntroCue;
    CuePointer pOutroCue;

    const QList<CuePointer> cues = m_pLoadedTrack->getCuePoints();
    for (const auto& pCue : cues) {
        switch (pCue->getType()) {
        case mixxx::CueType::MainCue:
            DEBUG_ASSERT(!pMainCue); // There should be only one MainCue cue
            pMainCue = pCue;
            break;
        case mixxx::CueType::Intro:
            DEBUG_ASSERT(!pIntroCue); // There should be only one Intro cue
            pIntroCue = pCue;
            break;
        case mixxx::CueType::Outro:
            DEBUG_ASSERT(!pOutroCue); // There should be only one Outro cue
            pOutroCue = pCue;
            break;
        case mixxx::CueType::HotCue:
        case mixxx::CueType::Loop: {
            if (pCue->getHotCue() == Cue::kNoHotCue) {
                continue;
            }

            const size_t hotcue = pCue->getHotCue();
            // Cue's hotcue doesn't have a hotcue control.
            if (hotcue >= m_hotcueControls.size()) {
                continue;
            }
            HotcueControl* pControl = m_hotcueControls.at(hotcue).get();


            CuePointer pOldCue(pControl->getCue());

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // old cue is detached if required
                attachCue(pCue, pControl);
            } else {
                // If the old hotcue is the same, then we only need to update
                Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
                pControl->setPosition(pos.startPosition);
                pControl->setEndPosition(pos.endPosition);
                pControl->setColor(pCue->getColor());
                pControl->setType(pCue->getType());
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
            break;
        }
        default:
            break;
        }
    }

    // Detach all hotcues that are no longer present
    for (int hotCueIndex = 0; hotCueIndex < kNumHotCues; ++hotCueIndex) {
        if (!active_hotcues.contains(hotCueIndex)) {
            HotcueControl* pControl = m_hotcueControls.at(hotCueIndex).get();
            detachCue(pControl);
        }
    }

    if (pIntroCue) {
        const auto startPosition = quantizeCuePoint(pIntroCue->getPosition());
        const auto endPosition = quantizeCuePoint(pIntroCue->getEndPosition());

        m_introStartPosition.set(startPosition.toEngineSamplePosMaybeInvalid());
        m_introStartEnabled.forceSet(startPosition.isValid());
        m_introEndPosition.set(endPosition.toEngineSamplePosMaybeInvalid());
        m_introEndEnabled.forceSet(endPosition.isValid());
    } else {
        m_introStartPosition.set(Cue::kNoPosition);
        m_introStartEnabled.forceSet(0.0);
        m_introEndPosition.set(Cue::kNoPosition);
        m_introEndEnabled.forceSet(0.0);
    }

    if (pOutroCue) {
        const auto startPosition = quantizeCuePoint(pOutroCue->getPosition());
        const auto endPosition = quantizeCuePoint(pOutroCue->getEndPosition());

        m_outroStartPosition.set(startPosition.toEngineSamplePosMaybeInvalid());
        m_outroStartEnabled.forceSet(startPosition.isValid());
        m_outroEndPosition.set(endPosition.toEngineSamplePosMaybeInvalid());
        m_outroEndEnabled.forceSet(endPosition.isValid());
    } else {
        m_outroStartPosition.set(Cue::kNoPosition);
        m_outroStartEnabled.forceSet(0.0);
        m_outroEndPosition.set(Cue::kNoPosition);
        m_outroEndEnabled.forceSet(0.0);
    }

    // Because of legacy, we store the main cue point twice and need to
    // sync both values.
    // The mixxx::CueType::MainCue from getCuePoints() has the priority
    mixxx::audio::FramePos mainCuePosition;
    if (pMainCue) {
        mainCuePosition = pMainCue->getPosition();
        // adjust the track cue accordingly
        m_pLoadedTrack->setMainCuePosition(mainCuePosition);
    } else {
        // If no load cue point is stored, read from track
        // Note: This is mixxx::audio::kStartFramePos for new tracks
        // and always a valid position.
        mainCuePosition = m_pLoadedTrack->getMainCuePosition();
        // A main cue point only needs to be added if the position
        // differs from the default position.
        if (mainCuePosition.isValid() &&
                mainCuePosition != mixxx::audio::kStartFramePos) {
            qInfo()
                    << "Adding missing main cue point at"
                    << mainCuePosition
                    << "for track"
                    << m_pLoadedTrack->getLocation();
            m_pLoadedTrack->createAndAddCue(
                    mixxx::CueType::MainCue,
                    Cue::kNoHotCue,
                    mainCuePosition,
                    mixxx::audio::kInvalidFramePos);
        }
    }

    DEBUG_ASSERT(mainCuePosition.isValid());
    const auto quantizedMainCuePosition = quantizeCuePoint(mainCuePosition);
    m_cuePoint.set(quantizedMainCuePosition.toEngineSamplePosMaybeInvalid());
}

void CueControl::trackAnalyzed() {
    if (frameInfo().currentPosition != m_usedSeekOnLoadPosition.getValue()) {
        // the track is already manual cued, don't re-cue
        return;
    }

    // Make track follow the updated cues.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadPreference();

    switch (seekOnLoadMode) {
    case SeekOnLoadMode::MainCue: {
        const auto position =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_cuePoint.get());
        if (position.isValid()) {
            seekOnLoad(position);
        }
        break;
    }
    case SeekOnLoadMode::IntroStart: {
        const auto position =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_introStartPosition.get());
        if (position.isValid()) {
            seekOnLoad(position);
        }
        break;
    }
    default:
        // nothing to do here
        break;
    }
}

void CueControl::trackCuesUpdated() {
    loadCuesFromTrack();
}

void CueControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    Q_UNUSED(pBeats);
    loadCuesFromTrack();
}

void CueControl::quantizeChanged(double v) {
    Q_UNUSED(v);

    // check if we were at the cue point before
    bool wasTrackAtCue = getTrackAt() == TrackAt::Cue;
    bool wasTrackAtIntro = isTrackAtIntroCue();

    loadCuesFromTrack();

    // if we are playing (no matter what reason for) do not seek
    if (m_pPlay->toBool()) {
        return;
    }

    // Retrieve new cue pos and follow
    const auto cuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (wasTrackAtCue && cuePosition.isValid()) {
        seekExact(cuePosition);
    }
    // Retrieve new intro start pos and follow
    const auto introPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    if (wasTrackAtIntro && introPosition.isValid()) {
        seekExact(introPosition);
    }
}

void CueControl::hotcueSet(HotcueControl* pControl, double value, HotcueSetMode mode) {
    //qDebug() << "CueControl::hotcueSet" << value;

    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://bugs.launchpad.net/mixxx/+bug/1653276
    hotcueClear(pControl, value);

    mixxx::audio::FramePos cueStartPosition;
    mixxx::audio::FramePos cueEndPosition;
    mixxx::CueType cueType = mixxx::CueType::Invalid;

    bool loopEnabled = m_loopEnabled.toBool();
    if (mode == HotcueSetMode::Auto) {
        mode = loopEnabled ? HotcueSetMode::Loop : HotcueSetMode::Cue;
    }

    switch (mode) {
    case HotcueSetMode::Cue: {
        // If no loop is enabled, just store regular jump cue
        cueStartPosition = getQuantizedCurrentPosition();
        cueType = mixxx::CueType::HotCue;
        break;
    }
    case HotcueSetMode::Loop: {
        if (loopEnabled) {
            // If a loop is enabled, save the current loop
            cueStartPosition =
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_loopStartPosition.get());
            cueEndPosition =
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_loopEndPosition.get());
        } else {
            // If no loop is enabled, save a loop starting from the current
            // position and with the current beatloop size
            cueStartPosition = getQuantizedCurrentPosition();
            double beatloopSize = m_beatLoopSize.get();
            const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
            if (beatloopSize <= 0 || !pBeats) {
                return;
            }
            if (cueStartPosition.isValid()) {
                cueEndPosition = pBeats->findNBeatsFromPosition(cueStartPosition, beatloopSize);
            }
        }
        cueType = mixxx::CueType::Loop;
        break;
    }
    default:
        DEBUG_ASSERT(!"Invalid HotcueSetMode");
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(cueType != mixxx::CueType::Invalid) {
        return;
    }

    // Abort if no position has been found.
    VERIFY_OR_DEBUG_ASSERT(cueStartPosition.isValid() &&
            (cueType != mixxx::CueType::Loop || cueEndPosition.isValid())) {
        return;
    }

    int hotcueIndex = pControl->getHotcueIndex();

    CuePointer pCue = m_pLoadedTrack->createAndAddCue(
            cueType,
            hotcueIndex,
            cueStartPosition,
            cueEndPosition);

    // TODO(XXX) deal with spurious signals
    attachCue(pCue, pControl);

    if (cueType == mixxx::CueType::Loop) {
        ConfigKey autoLoopColorsKey("[Controls]", "auto_loop_colors");
        if (getConfig()->getValue(autoLoopColorsKey, false)) {
            auto hotcueColorPalette =
                    m_colorPaletteSettings.getHotcueColorPalette();
            pCue->setColor(hotcueColorPalette.colorForHotcueIndex(hotcueIndex));
        } else {
            pCue->setColor(mixxx::PredefinedColorPalettes::kDefaultLoopColor);
        }
    } else {
        ConfigKey autoHotcueColorsKey("[Controls]", "auto_hotcue_colors");
        if (getConfig()->getValue(autoHotcueColorsKey, false)) {
            auto hotcueColorPalette =
                    m_colorPaletteSettings.getHotcueColorPalette();
            pCue->setColor(hotcueColorPalette.colorForHotcueIndex(hotcueIndex));
        } else {
            pCue->setColor(mixxx::PredefinedColorPalettes::kDefaultCueColor);
        }
    }

    if (cueType == mixxx::CueType::Loop) {
        setCurrentSavedLoopControlAndActivate(pControl);
    }

    // If quantize is enabled and we are not playing, jump to the cue point
    // since it's not necessarily where we currently are. TODO(XXX) is this
    // potentially invalid for vinyl control?
    bool playing = m_pPlay->toBool();
    if (!playing && m_pQuantizeEnabled->toBool()) {
        lock.unlock(); // prevent deadlock.
        // Enginebuffer will quantize more exactly than we can.
        seekAbs(cueStartPosition);
    }
}

void CueControl::hotcueGoto(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }
    const mixxx::audio::FramePos position = pControl->getPosition();
    if (position.isValid()) {
        seekAbs(position);
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    const mixxx::audio::FramePos position = pControl->getPosition();
    if (!position.isValid()) {
        return;
    }

    if (m_currentlyPreviewingIndex == Cue::kNoHotCue) {
        m_pPlay->set(0.0);
        seekExact(position);
    } else {
        // this becomes a play latch command if we are previewing
        m_pPlay->set(0.0);
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }
    const mixxx::audio::FramePos position = pControl->getPosition();
    if (position.isValid()) {
        seekAbs(position);
        // End previewing to not jump back if a sticking finger on a cue
        // button is released (just in case)
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        if (!m_pPlay->toBool()) {
            // don't move the cue point to the hot cue point in DENON mode
            m_bypassCueSetByPlay = true;
            m_pPlay->set(1.0);
        }
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueGotoAndLoop(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }
    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    const mixxx::audio::FramePos startPosition = pCue->getPosition();
    if (!startPosition.isValid()) {
        return;
    }

    if (pCue->getType() == mixxx::CueType::Loop) {
        seekAbs(startPosition);
        setCurrentSavedLoopControlAndActivate(pControl);
    } else if (pCue->getType() == mixxx::CueType::HotCue) {
        seekAbs(startPosition);
        setBeatLoop(startPosition, true);
    } else {
        return;
    }

    // End previewing to not jump back if a sticking finger on a cue
    // button is released (just in case)
    updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
    if (!m_pPlay->toBool()) {
        // don't move the cue point to the hot cue point in DENON mode
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueCueLoop(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    CuePointer pCue = pControl->getCue();

    if (!pCue || !pCue->getPosition().isValid()) {
        hotcueSet(pControl, value, HotcueSetMode::Cue);
        pCue = pControl->getCue();
        VERIFY_OR_DEBUG_ASSERT(pCue && pCue->getPosition().isValid()) {
            return;
        }
    }

    switch (pCue->getType()) {
    case mixxx::CueType::Loop: {
        // The hotcue_X_cueloop CO was invoked for a saved loop, set it as
        // active the first time this happens and toggle the loop_enabled state
        // on subsequent invocations.
        if (m_pCurrentSavedLoopControl != pControl) {
            setCurrentSavedLoopControlAndActivate(pControl);
        } else {
            bool loopActive = pControl->getStatus() == HotcueControl::Status::Active;
            Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
            setLoop(pos.startPosition, pos.endPosition, !loopActive);
        }
    } break;
    case mixxx::CueType::HotCue: {
        // The hotcue_X_cueloop CO was invoked for a hotcue. In that case,
        // create a beatloop starting at the hotcue position. This is useful for
        // mapping the CUE LOOP mode labeled on some controllers.
        setCurrentSavedLoopControlAndActivate(nullptr);
        const mixxx::audio::FramePos startPosition = pCue->getPosition();
        const bool loopActive = m_loopEnabled.toBool() &&
                startPosition ==
                        mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                                m_loopStartPosition.get());
        setBeatLoop(startPosition, !loopActive);
        break;
    }
    default:
        return;
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueActivate(HotcueControl* pControl, double value, HotcueSetMode mode) {
    //qDebug() << "CueControl::hotcueActivate" << value;

    CuePointer pCue = pControl->getCue();
    if (value > 0) {
        // pressed
        if (pCue && pCue->getPosition().isValid() &&
                pCue->getType() != mixxx::CueType::Invalid) {
            if (m_pPlay->toBool() && m_currentlyPreviewingIndex == Cue::kNoHotCue) {
                // playing by Play button
                switch (pCue->getType()) {
                case mixxx::CueType::HotCue:
                    hotcueGoto(pControl, value);
                    break;
                case mixxx::CueType::Loop:
                    if (m_pCurrentSavedLoopControl != pControl) {
                        setCurrentSavedLoopControlAndActivate(pControl);
                    } else {
                        bool loopActive = pControl->getStatus() ==
                                HotcueControl::Status::Active;
                        Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
                        setLoop(pos.startPosition, pos.endPosition, !loopActive);
                    }
                    break;
                default:
                    DEBUG_ASSERT(!"Invalid CueType!");
                }
            } else {
                // pressed during pause or preview
                hotcueActivatePreview(pControl, value);
            }
        } else {
            // pressed a not existing cue
            hotcueSet(pControl, value, mode);
        }
    } else {
        // released
        hotcueActivatePreview(pControl, value);
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueActivatePreview(HotcueControl* pControl, double value) {
    CuePointer pCue = pControl->getCue();
    int index = pControl->getHotcueIndex();
    if (value > 0) {
        if (m_currentlyPreviewingIndex != index) {
            pControl->cachePreviewingStartState();
            const mixxx::audio::FramePos position = pControl->getPreviewingPosition();
            mixxx::CueType type = pControl->getPreviewingType();
            if (type != mixxx::CueType::Invalid && position.isValid()) {
                updateCurrentlyPreviewingIndex(index);
                m_bypassCueSetByPlay = true;
                if (type == mixxx::CueType::Loop) {
                    setCurrentSavedLoopControlAndActivate(pControl);
                } else if (pControl->getStatus() == HotcueControl::Status::Set) {
                    pControl->setStatus(HotcueControl::Status::Active);
                }
                seekAbs(position);
                m_pPlay->set(1.0);
            }
        }
    } else if (m_currentlyPreviewingIndex == index) {
        // This is a release of a previewing hotcue
        const mixxx::audio::FramePos position = pControl->getPreviewingPosition();
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        if (position.isValid()) {
            seekExact(position);
        }
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::updateCurrentlyPreviewingIndex(int hotcueIndex) {
    int oldPreviewingIndex = m_currentlyPreviewingIndex.fetchAndStoreRelease(hotcueIndex);
    if (oldPreviewingIndex >= 0 && oldPreviewingIndex < kNumHotCues) {
        // We where already in previewing state, clean up ..
        HotcueControl* pLastControl = m_hotcueControls.at(oldPreviewingIndex).get();
        mixxx::CueType lastType = pLastControl->getPreviewingType();
        if (lastType == mixxx::CueType::Loop) {
            m_loopEnabled.set(0);
        }
        CuePointer pLastCue(pLastControl->getCue());
        if (pLastCue && pLastCue->getType() != mixxx::CueType::Invalid) {
            pLastControl->setStatus(HotcueControl::Status::Set);
        }
    }
}

void CueControl::hotcueClear(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }
    detachCue(pControl);
    m_pLoadedTrack->removeCue(pCue);
    setHotcueFocusIndex(Cue::kNoHotCue);
}

void CueControl::hotcuePositionChanged(
        HotcueControl* pControl, double value) {
    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    const auto newPosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);
    // Setting the position to Cue::kNoPosition is the same as calling hotcue_x_clear
    if (!newPosition.isValid()) {
        detachCue(pControl);
        return;
    }

    // TODO: Remove this check if we support positions < 0
    const auto trackEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pTrackSamples->get());
    if (newPosition <= mixxx::audio::kStartFramePos ||
            (trackEndPosition.isValid() && newPosition >= trackEndPosition)) {
        return;
    }

    if (pCue->getType() == mixxx::CueType::Loop && newPosition >= pCue->getEndPosition()) {
        return;
    }
    pCue->setStartPosition(newPosition);
}

void CueControl::hotcueEndPositionChanged(
        HotcueControl* pControl, double value) {
    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    const auto newEndPosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);

    // Setting the end position of a loop cue to Cue::kNoPosition converts
    // it into a regular jump cue
    if (pCue->getType() == mixxx::CueType::Loop && !newEndPosition.isValid()) {
        pCue->setType(mixxx::CueType::HotCue);
        pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
    } else {
        const mixxx::audio::FramePos startPosition = pCue->getPosition();
        if (startPosition.isValid() && newEndPosition > startPosition) {
            pCue->setEndPosition(newEndPosition);
        }
    }
}

void CueControl::hintReader(gsl::not_null<HintVector*> pHintList) {
    appendCueHint(pHintList, m_cuePoint.get(), Hint::Type::MainCue);

    // this is called from the engine thread
    // it is no locking required, because m_hotcueControl is filled during the
    // constructor and getPosition()->get() is a ControlObject
    for (const auto& pControl : qAsConst(m_hotcueControls)) {
        appendCueHint(pHintList, pControl->getPosition(), Hint::Type::HotCue);
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    if (pLoadedTrack) {
        CuePointer pN60dBSound =
                pLoadedTrack->findCueByType(mixxx::CueType::N60dBSound);
        if (pN60dBSound) {
            const mixxx::audio::FramePos frame = pN60dBSound->getPosition();
            appendCueHint(pHintList, frame, Hint::Type::FirstSound);
        }
    }
    appendCueHint(pHintList, m_introStartPosition.get(), Hint::Type::IntroStart);
    appendCueHint(pHintList, m_introEndPosition.get(), Hint::Type::IntroEnd);
    appendCueHint(pHintList, m_outroStartPosition.get(), Hint::Type::OutroStart);
}

// Moves the cue point to current position or to closest beat in case
// quantize is enabled
void CueControl::cueSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    // The m_pCuePoint CO is set via loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        pLoadedTrack->setMainCuePosition(position);
    }
}

void CueControl::cueClear(double value) {
    if (value <= 0) {
        return;
    }

    // the m_pCuePoint CO is set via loadCuesFromTrack()
    // no locking required
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    if (pLoadedTrack) {
        pLoadedTrack->setMainCuePosition(mixxx::audio::kStartFramePos);
    }
}

void CueControl::cueGoto(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    // Seek to cue point
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());

    // Note: We do not mess with play here, we continue playing or previewing.

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (mainCuePosition.isValid()) {
        seekAbs(mainCuePosition);
    }
}

void CueControl::cueGotoAndPlay(double value) {
    if (value <= 0) {
        return;
    }

    cueGoto(value);
    auto lock = lockMutex(&m_trackMutex);
    // Start playing if not already

    // End previewing to not jump back if a sticking finger on a cue
    // button is released (just in case)
    updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
    if (!m_pPlay->toBool()) {
        // don't move the cue point to the hot cue point in DENON mode
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }
}

void CueControl::cueGotoAndStop(double value) {
    if (value <= 0) {
        return;
    }

    if (m_currentlyPreviewingIndex == Cue::kNoHotCue) {
        m_pPlay->set(0.0);
        const auto mainCuePosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_cuePoint.get());
        if (mainCuePosition.isValid()) {
            seekExact(mainCuePosition);
        }
    } else {
        // this becomes a play latch command if we are previewing
        m_pPlay->set(0.0);
    }
}

void CueControl::cuePreview(double value) {
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            return;
        }

        updateCurrentlyPreviewingIndex(kMainCueIndex);
        seekAbs(mainCuePosition);
        m_pPlay->set(1.0);
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(mainCuePosition);
    }
}

void CueControl::cueCDJ(double value) {
    // This is how Pioneer cue buttons work:
    // If pressed while freely playing (i.e. playing and platter NOT being touched), stop playback and go to cue.
    // If pressed while NOT freely playing (i.e. stopped or playing but platter IS being touched), set new cue point.
    // If pressed while stopped and at cue, play while pressed.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    const auto freely_playing =
            m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(mainCuePosition);
        } else if (freely_playing || trackAt == TrackAt::End) {
            // Jump to cue when playing or when at end position
            m_pPlay->set(0.0);
            seekAbs(mainCuePosition);
        } else if (trackAt == TrackAt::Cue) {
            // paused at cue point
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            m_pPlay->set(1.0);
        } else {
            // Paused not at cue point and not at end position
            cueSet(value);

            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                // We need to re-get the cue point since it changed.
                const auto newCuePosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_cuePoint.get());
                if (newCuePosition.isValid()) {
                    // Enginebuffer will quantize more exactly than we can.
                    seekAbs(newCuePosition);
                }
            }
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        // Need to unlock before emitting any signals to prevent deadlock.
        seekExact(mainCuePosition);
    }

    // indicator may flash because the delayed adoption of seekAbs
    // Correct the Indicator set via play
    if (m_pLoadedTrack && !freely_playing) {
        m_pueIndicator.setBlinkValue(ControlIndicator::ON);
    } else {
        m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
    }
}

void CueControl::cueDenon(double value) {
    // This is how Denon DN-S 3700 cue buttons work:
    // If pressed go to cue and stop.
    // If pressed while stopped and at cue, play while pressed.
    // Cue Point is moved by play from pause

    bool playing = (m_pPlay->toBool());
    TrackAt trackAt = getTrackAt();

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(mainCuePosition);
        } else if (!playing && trackAt == TrackAt::Cue) {
            // paused at cue point
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            m_pPlay->set(1.0);
        } else {
            m_pPlay->set(0.0);
            seekExact(mainCuePosition);
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(mainCuePosition);
    }
}

void CueControl::cuePlay(double value) {
    // This is how CUP button works:
    // If freely playing (i.e. playing and platter NOT being touched), press to go to cue and stop.
    // If not freely playing (i.e. stopped or platter IS being touched), press to go to cue and stop.
    // On release, start playing from cue point.

    const auto freely_playing =
            m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    // pressed
    if (value > 0) {
        if (freely_playing) {
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            seekAbs(mainCuePosition);
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(value);
            // Just in case.
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                const auto newCuePosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_cuePoint.get());
                if (newCuePosition.isValid()) {
                    // Enginebuffer will quantize more exactly than we can.
                    seekAbs(newCuePosition);
                }
            }
        }
    } else if (trackAt == TrackAt::Cue) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(1.0);
    }
}

void CueControl::cueDefault(double v) {
    double cueMode = m_cueMode.get();
    // Decide which cue implementation to call based on the user preference
    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        cueDenon(v);
    } else if (cueMode == CUE_MODE_CUP) {
        cuePlay(v);
    } else {
        // The modes CUE_MODE_PIONEER and CUE_MODE_MIXXX are similar
        // are handled inside cueCDJ(v)
        // default to Pioneer mode
        cueCDJ(v);
    }
}

void CueControl::pause(double v) {
    auto lock = lockMutex(&m_trackMutex);
    //qDebug() << "CueControl::pause()" << v;
    if (v > 0.0) {
        m_pPlay->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    auto lock = lockMutex(&m_trackMutex);
    //qDebug() << "playStutter" << v;
    if (v > 0.0) {
        if (m_pPlay->toBool()) {
            if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
                // latch playing
                updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            } else {
                // Stutter
                cueGoto(1.0);
            }
        } else {
            m_pPlay->set(1.0);
        }
    }
}

void CueControl::introStartSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place intro start cue on or after
    // other intro/outro cues.
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introEndPosition.get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroStartPosition.get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroEndPosition.get());
    if (introEnd.isValid() && position >= introEnd) {
        qWarning()
                << "Trying to place intro start cue on or after intro end cue.";
        return;
    }
    if (outroStart.isValid() && position >= outroStart) {
        qWarning() << "Trying to place intro start cue on or after outro start "
                      "cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
        qWarning()
                << "Trying to place intro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Intro,
                    Cue::kNoHotCue,
                    position,
                    introEnd);
        } else {
            pCue->setStartAndEndPosition(position, introEnd);
        }
    }
}

void CueControl::introStartClear(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto introEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introEndPosition.get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introEndPosition.isValid()) {
            pCue->setStartPosition(mixxx::audio::kInvalidFramePos);
            pCue->setEndPosition(introEndPosition);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introStartActivate(double value) {
    if (value <= 0) {
        return;
    }

    const auto introStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    if (introStartPosition.isValid()) {
        seekAbs(introStartPosition);
    } else {
        introStartSet(1.0);
    }
}

void CueControl::introEndSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place intro end cue on or before
    // intro start cue, or on or after outro start/end cue.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroStartPosition.get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroEndPosition.get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place intro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (outroStart.isValid() && position >= outroStart) {
        qWarning()
                << "Trying to place intro end cue on or after outro start cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
        qWarning()
                << "Trying to place intro end cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Intro,
                    Cue::kNoHotCue,
                    introStart,
                    position);
        } else {
            pCue->setStartAndEndPosition(introStart, position);
        }
    }
}

void CueControl::introEndClear(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introStart.isValid()) {
            pCue->setStartPosition(introStart);
            pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introEndActivate(double value) {
    if (value == 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introEndPosition.get());
    lock.unlock();

    if (introEnd.isValid()) {
        seekAbs(introEnd);
    } else {
        introEndSet(1.0);
    }
}

void CueControl::outroStartSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place outro start cue on or before
    // intro end cue or on or after outro end cue.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introEndPosition.get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroEndPosition.get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place outro start cue on or before intro "
                      "start cue.";
        return;
    }
    if (introEnd.isValid() && position <= introEnd) {
        qWarning() << "Trying to place outro start cue on or before intro end "
                      "cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
        qWarning()
                << "Trying to place outro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Outro,
                    Cue::kNoHotCue,
                    position,
                    outroEnd);
        } else {
            pCue->setStartAndEndPosition(position, outroEnd);
        }
    }
}

void CueControl::outroStartClear(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroEndPosition.get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroEnd.isValid()) {
            pCue->setStartPosition(mixxx::audio::kInvalidFramePos);
            pCue->setEndPosition(outroEnd);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroStartActivate(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroStartPosition.get());
    lock.unlock();

    if (outroStart.isValid()) {
        seekAbs(outroStart);
    } else {
        outroStartSet(1.0);
    }
}

void CueControl::outroEndSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place outro end cue on or before
    // other intro/outro cues.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introEndPosition.get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroStartPosition.get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place outro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (introEnd.isValid() && position <= introEnd) {
        qWarning()
                << "Trying to place outro end cue on or before intro end cue.";
        return;
    }
    if (outroStart.isValid() && position <= outroStart) {
        qWarning() << "Trying to place outro end cue on or before outro start "
                      "cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Outro,
                    Cue::kNoHotCue,
                    outroStart,
                    position);
        } else {
            pCue->setStartAndEndPosition(outroStart, position);
        }
    }
}

void CueControl::outroEndClear(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroStartPosition.get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroStart.isValid()) {
            pCue->setStartPosition(outroStart);
            pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroEndActivate(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_outroEndPosition.get());
    lock.unlock();

    if (outroEnd.isValid()) {
        seekAbs(outroEnd);
    } else {
        outroEndSet(1.0);
    }
}

// This is also called from the engine thread. No locking allowed.
bool CueControl::updateIndicatorsAndModifyPlay(
        bool newPlay, bool oldPlay, bool playPossible) {
    //qDebug() << "updateIndicatorsAndModifyPlay" << newPlay << playPossible
    //        << m_iCurrentlyPreviewingHotcues << m_bPreviewing;
    CueMode cueMode = static_cast<CueMode>(static_cast<int>(m_cueMode.get()));
    if ((cueMode == CueMode::Denon || cueMode == CueMode::Numark) &&
            newPlay && !oldPlay && playPossible &&
            !m_bypassCueSetByPlay) {
        // in Denon mode each play from pause moves the cue point
        // if not previewing
        cueSet(1.0);
    }
    m_bypassCueSetByPlay = false;

    // when previewing, "play" was set by cue button, a following toggle request
    // (play = 0.0) is used for latching play.
    bool previewing = false;
    if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
        if (!newPlay && oldPlay) {
            // play latch request: stop previewing and go into normal play mode.
            int oldPreviewingIndex =
                    m_currentlyPreviewingIndex.fetchAndStoreRelease(
                            Cue::kNoHotCue);
            if (oldPreviewingIndex >= 0 && oldPreviewingIndex < kNumHotCues) {
                HotcueControl* pLastControl = m_hotcueControls.at(oldPreviewingIndex).get();
                mixxx::CueType lastType = pLastControl->getPreviewingType();
                if (lastType != mixxx::CueType::Loop) {
                    CuePointer pLastCue(pLastControl->getCue());
                    if (pLastCue && pLastCue->getType() != mixxx::CueType::Invalid) {
                        pLastControl->setStatus(HotcueControl::Status::Set);
                    }
                }
            }
            newPlay = true;
            m_playLatched.forceSet(1.0);
        } else {
            previewing = true;
            m_playLatched.forceSet(0.0);
        }
    }

    TrackAt trackAt = getTrackAt();

    if (!playPossible) {
        // play not possible
        newPlay = false;
        m_playIndicator.setBlinkValue(ControlIndicator::OFF);
        m_pStopButton->set(0.0);
        m_playLatched.forceSet(0.0);
    } else if (newPlay && !previewing) {
        // Play: Indicates a latched Play
        m_playIndicator.setBlinkValue(ControlIndicator::ON);
        m_pStopButton->set(0.0);
        m_playLatched.forceSet(1.0);
    } else {
        // Pause:
        m_pStopButton->set(1.0);
        m_playLatched.forceSet(0.0);
        if (cueMode == CueMode::Denon) {
            if (trackAt == TrackAt::Cue || previewing) {
                m_playIndicator.setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_playIndicator.setBlinkValue(
                        ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CueMode::Mixxx ||
                cueMode == CueMode::MixxxNoBlinking ||
                cueMode == CueMode::Numark) {
            m_playIndicator.setBlinkValue(ControlIndicator::OFF);
        } else {
            // Flashing indicates that play is possible in Pioneer mode
            m_playIndicator.setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
        }
    }

    if (cueMode != CueMode::Denon && cueMode != CueMode::Numark) {
        if (m_cuePoint.get() != Cue::kNoPosition) {
            if (newPlay == 0.0 && trackAt == TrackAt::ElseWhere) {
                if (cueMode == CueMode::Mixxx) {
                    // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                    m_pueIndicator.setBlinkValue(
                            ControlIndicator::RATIO1TO1_500MS);
                } else if (cueMode == CueMode::MixxxNoBlinking) {
                    m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
                } else {
                    // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                    m_pueIndicator.setBlinkValue(
                            ControlIndicator::RATIO1TO1_250MS);
                }
            } else {
                m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
        }
    }
    m_playStutter.set(newPlay ? 1.0 : 0.0);

    return newPlay;
}

// called from the engine thread
void CueControl::updateIndicators() {
    // No need for mutex lock because we are only touching COs.
    double cueMode = m_cueMode.get();
    TrackAt trackAt = getTrackAt();

    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        // Cue button is only lit at cue point
        bool playing = m_pPlay->toBool();
        if (trackAt == TrackAt::Cue) {
            // at cue point
            if (!playing) {
                m_pueIndicator.setBlinkValue(ControlIndicator::ON);
                m_playIndicator.setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
            if (!playing) {
                if (trackAt != TrackAt::End && cueMode != CUE_MODE_NUMARK) {
                    // Play will move cue point
                    m_playIndicator.setBlinkValue(
                            ControlIndicator::RATIO1TO1_500MS);
                } else {
                    // At track end
                    m_playIndicator.setBlinkValue(ControlIndicator::OFF);
                }
            }
        }
    } else {
        // Here we have CUE_MODE_PIONEER or CUE_MODE_MIXXX
        // default to Pioneer mode
        if (m_currentlyPreviewingIndex != kMainCueIndex) {
            const auto freely_playing =
                    m_pPlay->toBool() && !getEngineBuffer()->getScratching();
            if (!freely_playing) {
                switch (trackAt) {
                case TrackAt::ElseWhere:
                    if (cueMode == CUE_MODE_MIXXX) {
                        // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                        m_pueIndicator.setBlinkValue(
                                ControlIndicator::RATIO1TO1_500MS);
                    } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                        m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
                    } else {
                        // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                        m_pueIndicator.setBlinkValue(
                                ControlIndicator::RATIO1TO1_250MS);
                    }
                    break;
                case TrackAt::End:
                    // At track end
                    m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
                    break;
                case TrackAt::Cue:
                    // Next Press is preview
                    m_pueIndicator.setBlinkValue(ControlIndicator::ON);
                    break;
                }
            } else {
                // Cue indicator should be off when freely playing
                m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            // Preview
            m_pueIndicator.setBlinkValue(ControlIndicator::ON);
        }
    }
}

void CueControl::resetIndicators() {
    m_pueIndicator.setBlinkValue(ControlIndicator::OFF);
    m_playIndicator.setBlinkValue(ControlIndicator::OFF);
}

CueControl::TrackAt CueControl::getTrackAt() const {
    FrameInfo info = frameInfo();
    // Note: current can be in the padded silence after the track end > total.
    if (info.trackEndPosition.isValid() && info.currentPosition >= info.trackEndPosition) {
        return TrackAt::End;
    }
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_cuePoint.get());
    if (mainCuePosition.isValid() && fabs(info.currentPosition - mainCuePosition) < 0.5) {
        return TrackAt::Cue;
    }
    return TrackAt::ElseWhere;
}

mixxx::audio::FramePos CueControl::getQuantizedCurrentPosition() {
    FrameInfo info = frameInfo();

    // Note: currentPos can be past the end of the track, in the padded
    // silence of the last buffer. This position might be not reachable in
    // a future runs, depending on the buffering.

    // Don't quantize if quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return info.currentPosition;
    }

    const auto closestBeat =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pClosestBeat->get());
    // Note: closestBeat can be an interpolated beat past the end of the track,
    // which cannot be reached.
    if (closestBeat.isValid() && info.trackEndPosition.isValid() &&
            closestBeat <= info.trackEndPosition) {
        return closestBeat;
    }

    return info.currentPosition;
}

mixxx::audio::FramePos CueControl::quantizeCuePoint(mixxx::audio::FramePos position) {
    // Don't quantize unset cues.
    if (!position.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    // We need to use m_pTrackSamples here because FrameInfo is set later by
    // the engine and not during EngineBuffer::slotTrackLoaded.
    const auto trackEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pTrackSamples->get());

    VERIFY_OR_DEBUG_ASSERT(trackEndPosition.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    // Don't quantize when quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return position;
    }

    if (position > trackEndPosition) {
        // This can happen if the track length has changed or the cue was set in the
        // the padded silence after the track.
        position = trackEndPosition;
    }

    const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
    if (!pBeats) {
        return position;
    }

    const auto quantizedPosition = pBeats->findClosestBeat(position);
    // The closest beat can be an unreachable interpolated beat past the end of
    // the track.
    if (quantizedPosition.isValid() && quantizedPosition <= trackEndPosition) {
        return quantizedPosition;
    }

    return position;
}

bool CueControl::isTrackAtIntroCue() {
    const auto introStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_introStartPosition.get());
    return introStartPosition.isValid() &&
            (fabs(frameInfo().currentPosition - introStartPosition) < 0.5);
}

SeekOnLoadMode CueControl::getSeekOnLoadPreference() {
    int configValue =
            getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"),
                    static_cast<int>(SeekOnLoadMode::IntroStart));
    return static_cast<SeekOnLoadMode>(configValue);
}

void CueControl::hotcueFocusColorPrev(double value) {
    if (value <= 0) {
        return;
    }

    int hotcueIndex = getHotcueFocusIndex();
    if (hotcueIndex < 0 || hotcueIndex >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueIndex).get();
    if (!pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = pCue->getColor();
    if (!color) {
        return;
    }

    ColorPalette colorPalette = m_colorPaletteSettings.getHotcueColorPalette();
    pCue->setColor(colorPalette.previousColor(*color));
}

void CueControl::hotcueFocusColorNext(double value) {
    if (value <= 0) {
        return;
    }

    int hotcueIndex = getHotcueFocusIndex();
    if (hotcueIndex < 0 || hotcueIndex >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueIndex).get();
    if (!pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = pCue->getColor();
    if (!color) {
        return;
    }

    ColorPalette colorPalette = m_colorPaletteSettings.getHotcueColorPalette();
    pCue->setColor(colorPalette.nextColor(*color));
}

void CueControl::setCurrentSavedLoopControlAndActivate(HotcueControl* pControl) {
    HotcueControl* pOldSavedLoopControl = m_pCurrentSavedLoopControl.fetchAndStoreAcquire(nullptr);
    if (pOldSavedLoopControl && pOldSavedLoopControl != pControl) {
        // Disable previous saved loop
        DEBUG_ASSERT(pOldSavedLoopControl->getStatus() != HotcueControl::Status::Empty);
        pOldSavedLoopControl->setStatus(HotcueControl::Status::Set);
    }

    if (!pControl) {
        return;
    }
    CuePointer pCue = pControl->getCue();
    VERIFY_OR_DEBUG_ASSERT(pCue) {
        return;
    }

    mixxx::CueType type = pCue->getType();
    Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();

    VERIFY_OR_DEBUG_ASSERT(
            type == mixxx::CueType::Loop &&
            pos.startPosition.isValid() &&
            pos.endPosition.isValid()) {
        return;
    }

    // Set new control as active
    setLoop(pos.startPosition, pos.endPosition, true);
    pControl->setStatus(HotcueControl::Status::Active);
    m_pCurrentSavedLoopControl.storeRelease(pControl);
}

void CueControl::slotLoopReset() {
    setCurrentSavedLoopControlAndActivate(nullptr);
}

void CueControl::slotLoopEnabledChanged(bool enabled) {
    HotcueControl* pSavedLoopControl = m_pCurrentSavedLoopControl;
    if (!pSavedLoopControl) {
        return;
    }

    DEBUG_ASSERT(pSavedLoopControl->getStatus() != HotcueControl::Status::Empty);
    DEBUG_ASSERT(pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getPosition() ==
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_loopStartPosition.get()));
    DEBUG_ASSERT(pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getEndPosition() ==
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_loopEndPosition.get()));

    if (enabled) {
        pSavedLoopControl->setStatus(HotcueControl::Status::Active);
    } else {
        pSavedLoopControl->setStatus(HotcueControl::Status::Set);
    }
}

void CueControl::slotLoopUpdated(mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition) {
    HotcueControl* pSavedLoopControl = m_pCurrentSavedLoopControl;
    if (!pSavedLoopControl) {
        return;
    }

    if (pSavedLoopControl->getStatus() != HotcueControl::Status::Active) {
        slotLoopReset();
        return;
    }

    CuePointer pCue = pSavedLoopControl->getCue();
    if (!pCue) {
        // this can happen if the cue is deleted while this slot is cued
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(pCue->getType() == mixxx::CueType::Loop) {
        setCurrentSavedLoopControlAndActivate(nullptr);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(startPosition.isValid() && endPosition.isValid() &&
            startPosition < endPosition) {
        return;
    }

    DEBUG_ASSERT(pSavedLoopControl->getStatus() == HotcueControl::Status::Active);
    pCue->setStartPosition(startPosition);
    pCue->setEndPosition(endPosition);
    DEBUG_ASSERT(pSavedLoopControl->getStatus() == HotcueControl::Status::Active);
}

void CueControl::setHotcueFocusIndex(int hotcueIndex) {
    m_hotcueFocus.set(hotcueIndexToHotcueNumber(hotcueIndex));
}

int CueControl::getHotcueFocusIndex() const {
    return hotcueNumberToHotcueIndex(static_cast<int>(m_hotcueFocus.get()));
}


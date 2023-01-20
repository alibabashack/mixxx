#include "engine/controls/hotcuecontrol.h"

#include "engine/controls/hotcue.h"

namespace {

// Helper function to convert control values (i.e. doubles) into RgbColor
// instances (or nullopt if value < 0). This happens by using the integer
// component as RGB color codes (e.g. 0xFF0000).
inline mixxx::RgbColor::optional_t doubleToRgbColor(double value) {
    if (value < 0) {
        return std::nullopt;
    }
    auto colorCode = static_cast<mixxx::RgbColor::code_t>(value);
    if (value != mixxx::RgbColor::validateCode(colorCode)) {
        return std::nullopt;
    }
    return mixxx::RgbColor::optional(colorCode);
}

} // namespace

ConfigKey HotcueControl::keyForControl(const QString& name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we don't have a hotcue_0
    key.item = QStringLiteral("hotcue_") +
            QString::number(hotcueIndexToHotcueNumber(m_hotcueIndex)) +
            QChar('_') + name;
    return key;
}

HotcueControl::HotcueControl(const QString& group, int hotcueIndex)
        : m_group(group),
          m_hotcueIndex(hotcueIndex),
          m_pCue(nullptr) {
    m_hotcuePosition = std::make_unique<ControlObject>(keyForControl(QStringLiteral("position")));
    connect(m_hotcuePosition.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcuePositionChanged,
            Qt::DirectConnection);
    m_hotcuePosition->set(Cue::kNoPosition);

    m_hotcueEndPosition = std::make_unique<ControlObject>(
            keyForControl(QStringLiteral("endposition")));
    connect(m_hotcueEndPosition.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueEndPositionChanged,
            Qt::DirectConnection);
    m_hotcueEndPosition->set(Cue::kNoPosition);

    m_pHotcueStatus = std::make_unique<ControlObject>(keyForControl(QStringLiteral("status")));
    m_pHotcueStatus->setReadOnly();

    // Add an alias for the legacy hotcue_X_enabled CO
    ControlDoublePrivate::insertAlias(keyForControl(QStringLiteral("enabled")),
            keyForControl(QStringLiteral("status")));

    m_hotcueType = std::make_unique<ControlObject>(keyForControl(QStringLiteral("type")));
    m_hotcueType->setReadOnly();

    // The rgba value  of the color assigned to this color.
    m_hotcueColor = std::make_unique<ControlObject>(keyForControl(QStringLiteral("color")));
    m_hotcueColor->connectValueChangeRequest(
            this,
            &HotcueControl::slotHotcueColorChangeRequest,
            Qt::DirectConnection);
    connect(m_hotcueColor.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueColorChanged,
            Qt::DirectConnection);

    m_hotcueSet = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("set")));
    connect(m_hotcueSet.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSet,
            Qt::DirectConnection);

    m_hotcueSetCue = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("setcue")));
    connect(m_hotcueSetCue.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSetCue,
            Qt::DirectConnection);

    m_hotcueSetLoop = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("setloop")));
    connect(m_hotcueSetLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSetLoop,
            Qt::DirectConnection);

    m_hotcueGoto = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("goto")));
    connect(m_hotcueGoto.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGoto,
            Qt::DirectConnection);

    m_hotcueGotoAndPlay = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandplay")));
    connect(m_hotcueGotoAndPlay.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndPlay,
            Qt::DirectConnection);

    m_hotcueGotoAndStop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandstop")));
    connect(m_hotcueGotoAndStop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndStop,
            Qt::DirectConnection);

    m_hotcueGotoAndLoop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandloop")));
    connect(m_hotcueGotoAndLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndLoop,
            Qt::DirectConnection);

    // Enable/disable the loop associated with this hotcue (either a saved loop
    // or a beatloop from the hotcue position if this is a regular hotcue).
    m_hotcueCueLoop = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("cueloop")));
    connect(m_hotcueCueLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueCueLoop,
            Qt::DirectConnection);

    m_hotcueActivate = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activate")));
    connect(m_hotcueActivate.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivate,
            Qt::DirectConnection);

    m_hotcueActivateCue = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activatecue")));
    connect(m_hotcueActivateCue.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivateCue,
            Qt::DirectConnection);

    m_hotcueActivateLoop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activateloop")));
    connect(m_hotcueActivateLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivateLoop,
            Qt::DirectConnection);

    m_hotcueActivatePreview = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activate_preview")));
    connect(m_hotcueActivatePreview.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivatePreview,
            Qt::DirectConnection);

    m_hotcueClear = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("clear")));
    connect(m_hotcueClear.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueClear,
            Qt::DirectConnection);

    m_previewingType.setValue(mixxx::CueType::Invalid);
    m_previewingPosition.setValue(mixxx::audio::kInvalidFramePos);
}

HotcueControl::~HotcueControl() = default;

void HotcueControl::slotHotcueSet(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Auto);
}

void HotcueControl::slotHotcueSetCue(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Cue);
}

void HotcueControl::slotHotcueSetLoop(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Loop);
}

void HotcueControl::slotHotcueGoto(double v) {
    emit hotcueGoto(this, v);
}

void HotcueControl::slotHotcueGotoAndPlay(double v) {
    emit hotcueGotoAndPlay(this, v);
}

void HotcueControl::slotHotcueGotoAndStop(double v) {
    emit hotcueGotoAndStop(this, v);
}

void HotcueControl::slotHotcueGotoAndLoop(double v) {
    emit hotcueGotoAndLoop(this, v);
}

void HotcueControl::slotHotcueCueLoop(double v) {
    emit hotcueCueLoop(this, v);
}

void HotcueControl::slotHotcueActivate(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Auto);
}

void HotcueControl::slotHotcueActivateCue(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Cue);
}

void HotcueControl::slotHotcueActivateLoop(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Loop);
}

void HotcueControl::slotHotcueActivatePreview(double v) {
    emit hotcueActivatePreview(this, v);
}

void HotcueControl::slotHotcueClear(double v) {
    emit hotcueClear(this, v);
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    emit hotcuePositionChanged(this, newPosition);
}

void HotcueControl::slotHotcueEndPositionChanged(double newEndPosition) {
    emit hotcueEndPositionChanged(this, newEndPosition);
}

void HotcueControl::slotHotcueColorChangeRequest(double color) {
    if (color < 0 || color > 0xFFFFFF) {
        qWarning() << "slotHotcueColorChanged got invalid value:" << color;
        return;
    }
    m_hotcueColor->setAndConfirm(color);
}

void HotcueControl::slotHotcueColorChanged(double newColor) {
    if (!m_pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = doubleToRgbColor(newColor);
    VERIFY_OR_DEBUG_ASSERT(color) {
        return;
    }

    m_pCue->setColor(*color);
    emit hotcueColorChanged(this, newColor);
}

mixxx::audio::FramePos HotcueControl::getPosition() const {
    return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_hotcuePosition->get());
}

mixxx::audio::FramePos HotcueControl::getEndPosition() const {
    return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_hotcueEndPosition->get());
}

void HotcueControl::setCue(const CuePointer& pCue) {
    DEBUG_ASSERT(!m_pCue);
    Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
    setPosition(pos.startPosition);
    setEndPosition(pos.endPosition);
    setColor(pCue->getColor());
    setStatus((pCue->getType() == mixxx::CueType::Invalid)
                    ? HotcueControl::Status::Empty
                    : HotcueControl::Status::Set);
    setType(pCue->getType());
    // set pCue only if all other data is in place
    // because we have a null check for valid data else where in the code
    m_pCue = pCue;
}
mixxx::RgbColor::optional_t HotcueControl::getColor() const {
    return doubleToRgbColor(m_hotcueColor->get());
}

void HotcueControl::setColor(mixxx::RgbColor::optional_t newColor) {
    if (newColor) {
        m_hotcueColor->set(*newColor);
    }
}
void HotcueControl::resetCue() {
    // clear pCue first because we have a null check for valid data else where
    // in the code
    m_pCue.reset();
    setPosition(mixxx::audio::kInvalidFramePos);
    setEndPosition(mixxx::audio::kInvalidFramePos);
    setType(mixxx::CueType::Invalid);
    setStatus(Status::Empty);
}

void HotcueControl::setPosition(mixxx::audio::FramePos position) {
    m_hotcuePosition->set(position.toEngineSamplePosMaybeInvalid());
}

void HotcueControl::setEndPosition(mixxx::audio::FramePos endPosition) {
    m_hotcueEndPosition->set(endPosition.toEngineSamplePosMaybeInvalid());
}

void HotcueControl::setType(mixxx::CueType type) {
    m_hotcueType->forceSet(static_cast<double>(type));
}

void HotcueControl::setStatus(HotcueControl::Status status) {
    m_pHotcueStatus->forceSet(static_cast<double>(status));
}

HotcueControl::Status HotcueControl::getStatus() const {
    // Cast to int before casting to the int-based enum class because MSVC will
    // throw a hissy fit otherwise.
    return static_cast<Status>(static_cast<int>(m_pHotcueStatus->get()));
}

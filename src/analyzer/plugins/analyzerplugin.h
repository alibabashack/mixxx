#pragma once

#include <QString>

#include "audio/frame.h"
#include "track/beats.h"
#include "track/bpm.h"
#include "track/keys.h"
#include "util/types.h"

namespace mixxx {

class AnalyzerPluginInfo {
  public:
    AnalyzerPluginInfo(const QString& id,
            const QString& author,
            const QString& name,
            bool isConstantTempoSupported)
            : m_id(id),
              m_author(author),
              m_name(name),
              m_isConstantTempoSupported(isConstantTempoSupported) {
    }

    [[nodiscard]] const QString& id() const {
        return m_id;
    }

    [[nodiscard]] const QString& author() const {
        return m_author;
    }

    [[nodiscard]] const QString& name() const {
        return m_name;
    }

    [[nodiscard]] bool isConstantTempoSupported() const {
        return m_isConstantTempoSupported;
    }

  private:
    QString m_id;
    QString m_author;
    QString m_name;
    bool m_isConstantTempoSupported;
};

class AnalyzerPlugin {
  public:
    virtual ~AnalyzerPlugin() = default;

    [[nodiscard]] virtual QString id() const {
        return info().id();
    }
    [[nodiscard]] virtual QString author() const {
        return info().author();
    }
    [[nodiscard]] virtual QString name() const {
        return info().name();
    }
    [[nodiscard]] virtual AnalyzerPluginInfo info() const = 0;

    virtual bool initialize(mixxx::audio::SampleRate sampleRate) = 0;
    virtual bool processSamples(const CSAMPLE* pIn, SINT iLen) = 0;
    virtual bool finalize() = 0;
};

class AnalyzerBeatsPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerBeatsPlugin() override = default;

    [[nodiscard]] virtual bool supportsBeatTracking() const = 0;
    [[nodiscard]] virtual mixxx::Bpm getBpm() const {
        return {};
    }
    [[nodiscard]] virtual QVector<mixxx::audio::FramePos> getBeats() const {
        return {};
    }
};

class AnalyzerKeyPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerKeyPlugin() override = default;

    [[nodiscard]] virtual KeyChangeList getKeyChanges() const = 0;
};

} // namespace mixxx

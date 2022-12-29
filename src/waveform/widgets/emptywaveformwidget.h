#pragma once

#include <QWidget>

#include "waveform/widgets/nonglwaveformwidgetabstract.h"

// This class can be used as a template file to create new WaveformWidgets it
// contain minimal set of method to re-implement

class EmptyWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~EmptyWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::EmptyWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Empty"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    EmptyWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

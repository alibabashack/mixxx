#pragma once

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLSimpleWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLSimpleWaveformWidget(const QString& group, QWidget* parent);
    ~GLSimpleWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSimpleWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Simple"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

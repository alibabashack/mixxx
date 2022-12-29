#pragma once

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLRGBWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLRGBWaveformWidget(const QString& group, QWidget* parent);
    ~GLRGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
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

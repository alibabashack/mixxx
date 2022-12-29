#pragma once

#include <QGLWidget>

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLWaveformWidget(const QString& group, QWidget* parent);
    ~GLWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLFilteredWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
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

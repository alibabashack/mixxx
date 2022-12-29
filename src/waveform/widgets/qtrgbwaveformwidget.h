#pragma once

#include "glwaveformwidgetabstract.h"

class QtRGBWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~QtRGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::QtRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    QtRGBWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

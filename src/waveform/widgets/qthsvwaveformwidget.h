#pragma once

#include "glwaveformwidgetabstract.h"

class QtHSVWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~QtHSVWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::QtHSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    QtHSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

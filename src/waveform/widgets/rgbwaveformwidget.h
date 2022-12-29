#pragma once

#include <QWidget>

#include "nonglwaveformwidgetabstract.h"

class RGBWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~RGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::RGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    RGBWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

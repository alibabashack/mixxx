#pragma once

#include <QWidget>

#include "nonglwaveformwidgetabstract.h"

class HSVWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~HSVWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::HSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    HSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

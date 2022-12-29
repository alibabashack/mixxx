#pragma once

#include <QWidget>

#include "nonglwaveformwidgetabstract.h"

class SoftwareWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    ~SoftwareWaveformWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::SoftwareWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    SoftwareWaveformWidget(const QString& groupp, QWidget* parent);
    friend class WaveformWidgetFactory;
};

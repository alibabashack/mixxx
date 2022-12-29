#pragma once

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLVSyncTestWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLVSyncTestWidget(const QString& group, QWidget* parent);
    ~GLVSyncTestWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

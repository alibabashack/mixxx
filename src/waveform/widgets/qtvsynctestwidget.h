#pragma once

#include "glwaveformwidgetabstract.h"

class QtVSyncTestWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtVSyncTestWidget(const QString& group, QWidget* parent);
    ~QtVSyncTestWidget() override;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::QtVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

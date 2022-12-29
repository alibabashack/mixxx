#pragma once

#include "waveform/renderers/glwaveformrenderer.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveform/renderers/glwaveformrenderer.h"

class ControlObject;

class GLWaveformRendererRGB : public GLWaveformRenderer {
  public:
    explicit GLWaveformRendererRGB(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLWaveformRendererRGB() override;

    void onSetup(const QDomNode& node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(GLWaveformRendererRGB);
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

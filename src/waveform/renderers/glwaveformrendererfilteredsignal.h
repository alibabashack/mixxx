#pragma once

#include "waveform/renderers/glwaveformrenderer.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveform/renderers/glwaveformrenderer.h"

QT_FORWARD_DECLARE_CLASS(QDomNode)
class ControlObject;

class GLWaveformRendererFilteredSignal : public GLWaveformRenderer {
  public:
    explicit GLWaveformRendererFilteredSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLWaveformRendererFilteredSignal() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

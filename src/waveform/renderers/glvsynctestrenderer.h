#pragma once

#include "waveform/renderers/glwaveformrenderer.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveform/renderers/glwaveformrenderer.h"

class ControlObject;

class GLVSyncTestRenderer : public GLWaveformRenderer {
  public:
    explicit GLVSyncTestRenderer(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    ~GLVSyncTestRenderer() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
private:
    int m_drawcount;
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

#pragma once

#include "waveformrenderersignalbase.h"

class ControlObject;

class QtVSyncTestRenderer : public WaveformRendererSignalBase {
  public:
    explicit QtVSyncTestRenderer(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~QtVSyncTestRenderer() override;

    void onSetup(const QDomNode &node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
  private:
    int m_drawcount;
};

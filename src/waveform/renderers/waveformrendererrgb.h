#pragma once

#include "util/class.h"
#include "waveformrenderersignalbase.h"

class WaveformRendererRGB : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererRGB() override;

    void onSetup(const QDomNode& node) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};

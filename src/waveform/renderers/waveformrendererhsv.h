#pragma once

#include "util/class.h"
#include "waveformrenderersignalbase.h"

class WaveformRendererHSV : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererHSV() override;

    void onSetup(const QDomNode& node) override;

    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererHSV);
};

#pragma once

#include <QColor>

#include "skin/legacy/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRendererPreroll : public WaveformRendererAbstract {
  public:
    explicit WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRendererPreroll() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    QColor m_color;
    
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};

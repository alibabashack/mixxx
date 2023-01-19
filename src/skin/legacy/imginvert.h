#pragma once

#include "imgsource.h"

class ImgInvert : public ImgColorProcessor {

public:
    inline ImgInvert(ImgSource* parent) : ImgColorProcessor(parent) {}
    [[nodiscard]] QColor doColorCorrection(const QColor& c) const override;
};

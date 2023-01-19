#pragma once

#include "imgsource.h"

class ImgLoader : public ImgSource {

public:
    ImgLoader();
    [[nodiscard]] QImage* getImage(const QString &fileName, double scaleFactor) const override;
};

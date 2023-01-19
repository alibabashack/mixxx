#pragma once

#include <QImage>
#include <QColor>
#include <QString>
#include <QRgb>
#include <QtDebug>

class ImgSource {
  public:
    virtual ~ImgSource() {};
    [[nodiscard]] virtual QImage* getImage(const QString& fileName, double scaleFactor) const = 0;
    [[nodiscard]] virtual QColor getCorrectColor(const QColor& c) const { return c; }
    virtual void correctImageColors(QImage* p) const { (void)p; };
    [[nodiscard]] virtual bool willCorrectColors() const { return false; };
};

class ImgProcessor : public ImgSource {

  public:
    ~ImgProcessor() override {};
    inline ImgProcessor(ImgSource* parent) : m_parent(parent) {}
    [[nodiscard]] virtual QColor doColorCorrection(const QColor& c) const = 0;
    [[nodiscard]] QColor getCorrectColor(const QColor& c) const override {
        return doColorCorrection(m_parent->getCorrectColor(c));
    }
    void correctImageColors(QImage* p) const override { (void)p; }
    [[nodiscard]] bool willCorrectColors() const override { return false; }

  protected:
    ImgSource* m_parent;
};

class ImgColorProcessor : public ImgProcessor {

public:
    ~ImgColorProcessor() override {};

    inline ImgColorProcessor(ImgSource* parent) : ImgProcessor(parent) {}

    [[nodiscard]] QImage* getImage(const QString& fileName, double scaleFactor) const override {
        QImage* i = m_parent->getImage(fileName, scaleFactor);
        correctImageColors(i);
        return i;
    }

    void correctImageColors(QImage* i) const override {
        if (i == nullptr || i->isNull()) {
            return;
        }

        QColor col;

        int bytesPerPixel = 4;

        switch(i->format()) {

        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        case QImage::Format_Indexed8:
            bytesPerPixel = 1;
            break;

        case QImage::Format_RGB16:
        case QImage::Format_RGB555:
        case QImage::Format_RGB444:
        case QImage::Format_ARGB4444_Premultiplied:
            bytesPerPixel = 2;
            break;

        case QImage::Format_ARGB8565_Premultiplied:
        case QImage::Format_RGB666:
        case QImage::Format_ARGB6666_Premultiplied:
        case QImage::Format_ARGB8555_Premultiplied:
        case QImage::Format_RGB888:
            bytesPerPixel = 3;
            break;

        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        case QImage::Format_RGB32:
            bytesPerPixel = 4;
            break;

        case QImage::Format_Invalid:
        default:
            bytesPerPixel = 0;
            break;
        }

        //qDebug() << "ImgColorProcessor working on "
        //         << img << " bpp: "
        //         << bytesPerPixel << " format: " << i->format();

        if (bytesPerPixel < 4) {
            // Handling Indexed color or mono colors requires different logic
            qDebug() << "ImgColorProcessor aborting on unsupported color format:"
                     << i->format();
            return;
        }

        for (int y = 0; y < i->height(); y++) {
            QRgb *line = (QRgb*)i->scanLine(y); // cast the returned pointer to QRgb*

            if (line == nullptr) {
                // Image is invalid.
                continue;
            }

            for (int x = 0; x < i->width(); x++) {
                col.setRgba(*line);
                col = doColorCorrection(col);
                *line = col.rgba();
                line++;
            }
        }
    }

    [[nodiscard]] bool willCorrectColors() const override { return true; }
};

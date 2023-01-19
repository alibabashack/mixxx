#pragma once

#include <QString>
#include <QByteArray>

// A class representing an image source for a pixmap
// A bundle of a file path, raw data or inline svg
class PixmapSource final {
  public:
    PixmapSource();
    PixmapSource(const QString& filepath);

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] bool isSVG() const;
    [[nodiscard]] bool isBitmap() const;
    void setSVG(const QByteArray& content);
    [[nodiscard]] const QString& getPath() const;
    [[nodiscard]] const QByteArray& getSvgSourceData() const;
    [[nodiscard]] QString getId() const;

  private:
    enum Type {
        SVG,
        BITMAP
    };

    QString m_path;
    QByteArray m_svgSourceData;
    enum Type m_eType;
};

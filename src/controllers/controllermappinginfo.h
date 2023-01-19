#pragma once

#include <QDomElement>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QString>

#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "preferences/usersettings.h"

struct ProductInfo {
    QString protocol;
    QString vendor_id;
    QString product_id;

    // HID-specific
    QString in_epaddr;
    QString out_epaddr;

    // Bulk-specific
    QString usage_page;
    QString usage;
    QString interface_number;
};

/// Base class handling enumeration and parsing of mapping info headers
///
/// This class handles enumeration and parsing of controller XML description file
/// <info> header tags. It can be used to match controllers automatically or to
/// show details for a mapping.
class MappingInfo {
  public:
    MappingInfo();
    MappingInfo(const QString& path);

    [[nodiscard]] inline bool isValid() const {
        return m_valid;
    }

    [[nodiscard]] inline const QString getPath() const {
        return m_path;
    }
    [[nodiscard]] inline const QString getDirPath() const {
        return m_dirPath;
    }
    [[nodiscard]] inline const QString getName() const {
        return m_name;
    }
    [[nodiscard]] inline const QString getDescription() const {
        return m_description;
    }
    [[nodiscard]] inline const QString getForumLink() const {
        return m_forumlink;
    }
    [[nodiscard]] inline const QString getWikiLink() const {
        return m_wikilink;
    }
    [[nodiscard]] inline const QString getAuthor() const {
        return m_author;
    }

    [[nodiscard]] inline const QList<ProductInfo>& getProducts() const {
        return m_products;
    }

  private:
    [[nodiscard]] ProductInfo parseBulkProduct(const QDomElement& element) const;
    [[nodiscard]] ProductInfo parseHIDProduct(const QDomElement& element) const;

    bool m_valid;
    QString m_path;
    QString m_dirPath;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_wikilink;
    QList<ProductInfo> m_products;
};

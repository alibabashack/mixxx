#pragma once

#include "util/assert.h"

#include <QUrl>

namespace mixxx {

class UrlResource {
  public:
    virtual ~UrlResource() = default;

    [[nodiscard]] QUrl getUrl() const {
        return m_url;
    }
    [[nodiscard]] QString getUrlString() const {
        return m_url.toString();
    }

  protected:
    explicit UrlResource(const QUrl& url)
            : m_url(url) {
    }

    [[nodiscard]] inline bool isLocalFile() const {
        // TODO(XXX): We need more testing how network shares are
        // handled! From the documentation of QUrl::isLocalFile():
        // "Note that this function considers URLs with hostnames
        // to be local file paths, ..."
        return m_url.isLocalFile();
    }

    [[nodiscard]] inline QString getLocalFileName() const {
        DEBUG_ASSERT(isLocalFile());
        return m_url.toLocalFile();
    }

  private:
    QUrl m_url;
};

} // namespace mixxx

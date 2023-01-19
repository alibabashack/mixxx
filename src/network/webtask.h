#pragma once

#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

#include "network/httpstatuscode.h"
#include "network/networktask.h"
#include "util/optional.h"
#include "util/performancetimer.h"

namespace mixxx {

namespace network {

class WebResponse final {
  public:
    static void registerMetaType();

    WebResponse()
            : m_statusCode(kHttpStatusCodeInvalid) {
    }
    explicit WebResponse(
            QUrl requestUrl,
            QUrl replyUrl = QUrl(),
            HttpStatusCode statusCode = kHttpStatusCodeInvalid)
            : m_requestUrl(std::move(requestUrl)),
              m_replyUrl(std::move(replyUrl)),
              m_statusCode(statusCode) {
    }
    WebResponse(const WebResponse&) = default;
    WebResponse(WebResponse&&) = default;

    WebResponse& operator=(const WebResponse&) = default;
    WebResponse& operator=(WebResponse&&) = default;

    [[nodiscard]] bool isStatusCodeSuccess() const {
        return HttpStatusCode_isSuccess(m_statusCode);
    }

    [[nodiscard]] const QUrl& requestUrl() const {
        return m_requestUrl;
    }

    [[nodiscard]] const QUrl& replyUrl() const {
        return m_replyUrl;
    }

    [[nodiscard]] HttpStatusCode statusCode() const {
        return m_statusCode;
    }

    friend QDebug operator<<(QDebug dbg, const WebResponse& arg);

  private:
    QUrl m_requestUrl;
    QUrl m_replyUrl;
    HttpStatusCode m_statusCode;
};

class WebResponseWithContent final {
  public:
    static void registerMetaType();

    WebResponseWithContent() = default;
    WebResponseWithContent(
            WebResponse&& response,
            QMimeType&& contentType,
            QByteArray&& contentData)
            : m_response(response),
              m_contentType(contentType),
              m_contentData(contentData) {
    }
    WebResponseWithContent(const WebResponseWithContent&) = default;
    WebResponseWithContent(WebResponseWithContent&&) = default;

    WebResponseWithContent& operator=(const WebResponseWithContent&) = default;
    WebResponseWithContent& operator=(WebResponseWithContent&&) = default;

    [[nodiscard]] bool isStatusCodeSuccess() const {
        return m_response.isStatusCodeSuccess();
    }

    [[nodiscard]] HttpStatusCode statusCode() const {
        return m_response.statusCode();
    }

    [[nodiscard]] const QUrl& requestUrl() const {
        return m_response.requestUrl();
    }

    [[nodiscard]] const QUrl& replyUrl() const {
        return m_response.replyUrl();
    }

    [[nodiscard]] const QMimeType& contentType() const {
        return m_contentType;
    }

    [[nodiscard]] const QByteArray& contentData() const {
        return m_contentData;
    }

    friend QDebug operator<<(QDebug dbg, const WebResponseWithContent& arg);

  private:
    WebResponse m_response;
    QMimeType m_contentType;
    QByteArray m_contentData;
};

/// A transient task for performing a single HTTP network request
/// asynchronously.
class WebTask : public NetworkTask {
    Q_OBJECT

  public:
    static QMimeType readContentType(
            const QNetworkReply& reply);
    static std::optional<QByteArray> readContentData(
            QNetworkReply* reply);

    explicit WebTask(
            QNetworkAccessManager* networkAccessManager,
            QObject* parent = nullptr);
    ~WebTask() override = default;

    [[nodiscard]] bool isBusy() const {
        return state() == State::Starting ||
                state() == State::Pending;
    }

  signals:
    /// Network or server-side abort/timeout/failure
    void networkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

  public slots:
    void slotStart(
            int timeoutMillis = kNoTimeout,
            int delayMillis = kNoStartDelay) override;
    void slotAbort() override;

  private slots:
    void slotNetworkReplyFinished();

  protected:
    void timerEvent(QTimerEvent* event) final;

    enum class State {
        // Initial state
        Initial,
        // Timed start
        Starting,
        // Pending state
        Pending,
        // Terminal states
        Aborted,
        TimedOut,
        Failed,
        Finished,
    };

    [[nodiscard]] State state() const {
        return m_state;
    }

    [[nodiscard]] bool hasTerminated() const {
        return state() == State::Aborted ||
                state() == State::TimedOut ||
                state() == State::Failed ||
                state() == State::Finished;
    }

    /// Send a signal after a failed network response.
    void emitNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const WebResponseWithContent& responseWithContent);

  protected:
    virtual void onNetworkError(
            QNetworkReply* pFinishedNetworkReply,
            HttpStatusCode statusCode);

  private:
    QUrl abortPendingNetworkReply();

    /// Try to compose and send the actual network request.
    /// Return nullptr on failure.
    virtual QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) = 0;

    /// Optional: Do something after aborted.
    virtual void doNetworkReplyAborted(
            QNetworkReply* abortedNetworkReply) {
        Q_UNUSED(abortedNetworkReply);
    }

    /// Handle network response.
    virtual void doNetworkReplyFinished(
            QNetworkReply* pFinishedNetworkReply,
            HttpStatusCode statusCode) = 0;

    /// Handle the abort and ensure that the task eventually
    /// gets deleted. The default implementation logs a warning
    /// and deletes the task.
    void onNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const WebResponseWithContent& responseWithContent);

    /// All member variables must only be accessed from
    /// the event loop thread!!

    State m_state;

    PerformanceTimer m_timer;

    int m_timeoutTimerId;
    int m_timeoutMillis;

    SafeQPointer<QNetworkReply> m_pendingNetworkReplyWeakPtr;
};

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::WebResponse);

Q_DECLARE_METATYPE(mixxx::network::WebResponseWithContent);

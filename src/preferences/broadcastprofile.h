#pragma once

#include <QSharedPointer>
#include <QObject>
#include <QString>

#include "preferences/usersettings.h"

class BroadcastProfile;
typedef QSharedPointer<BroadcastProfile> BroadcastProfilePtr;
Q_DECLARE_METATYPE(BroadcastProfilePtr)

class BroadcastProfile : public QObject {
  Q_OBJECT

  public:
    enum StatusStates {
          STATUS_UNCONNECTED = 0, // IDLE state, no error
          STATUS_CONNECTING = 1, // 30 s max
          STATUS_CONNECTED = 2, // On Air
          STATUS_FAILURE = 3 // Happens when disconnected by an error
    };

    explicit BroadcastProfile(const QString& profileName,
                              QObject* parent = nullptr);
    bool save(const QString& filename);
    bool equals(BroadcastProfilePtr other);
    bool valuesEquals(BroadcastProfilePtr other);
    BroadcastProfilePtr valuesCopy();
    void copyValuesTo(BroadcastProfilePtr other);

    static BroadcastProfilePtr loadFromFile(const QString& filename);
    static bool validName(const QString& str);
    static QString stripForbiddenChars(const QString& str);

    [[nodiscard]] QString getLastFilename() const;

    void setConnectionStatus(int newState);
    int connectionStatus();

    void setSecureCredentialStorage(bool enabled);
    bool secureCredentialStorage();

    void setProfileName(const QString& profileName);
    [[nodiscard]] QString getProfileName() const;

    [[nodiscard]] bool getEnabled() const;
    void setEnabled(bool value);

    [[nodiscard]] QString getHost() const;
    void setHost(const QString& value);

    [[nodiscard]] int getPort() const;
    void setPort(int value);

    [[nodiscard]] QString getServertype() const;
    void setServertype(const QString& value);

    [[nodiscard]] QString getLogin() const;
    void setLogin(const QString& value);

    [[nodiscard]] QString getPassword() const;
    void setPassword(const QString& value);

    [[nodiscard]] bool getEnableReconnect() const;
    void setEnableReconnect(bool value);

    [[nodiscard]] double getReconnectPeriod() const;
    void setReconnectPeriod(double value);

    [[nodiscard]] bool getLimitReconnects() const;
    void setLimitReconnects(bool value);

    [[nodiscard]] int getMaximumRetries() const;
    void setMaximumRetries(int value);

    [[nodiscard]] bool getNoDelayFirstReconnect() const;
    void setNoDelayFirstReconnect(bool value);

    [[nodiscard]] double getReconnectFirstDelay() const;
    void setReconnectFirstDelay(double value);

    [[nodiscard]] QString getFormat() const;
    void setFormat(const QString& value);

    [[nodiscard]] int getBitrate() const;
    void setBitrate(int value);

    [[nodiscard]] int getChannels() const;
    void setChannels(int value);

    [[nodiscard]] QString getMountpoint() const;
    void setMountPoint(const QString& value);

    [[nodiscard]] QString getStreamName() const;
    void setStreamName(const QString& value);

    [[nodiscard]] QString getStreamDesc() const;
    void setStreamDesc(const QString& value);

    [[nodiscard]] QString getStreamGenre() const;
    void setStreamGenre(const QString& value);

    [[nodiscard]] bool getStreamPublic() const;
    void setStreamPublic(bool value);

    [[nodiscard]] QString getStreamWebsite() const;
    void setStreamWebsite(const QString& value);

    [[nodiscard]] QString getStreamIRC() const;
    void setStreamIRC(const QString& value);

    [[nodiscard]] QString getStreamAIM() const;
    void setStreamAIM(const QString& value);

    [[nodiscard]] QString getStreamICQ() const;
    void setStreamICQ(const QString& value);

    [[nodiscard]] bool getEnableMetadata() const;
    void setEnableMetadata(bool value);

    [[nodiscard]] QString getMetadataCharset() const;
    void setMetadataCharset(const QString& value);

    [[nodiscard]] QString getCustomArtist() const;
    void setCustomArtist(const QString& value);

    [[nodiscard]] QString getCustomTitle() const;
    void setCustomTitle(const QString& value);

    [[nodiscard]] QString getMetadataFormat() const;
    void setMetadataFormat(const QString& value);

    [[nodiscard]] bool getOggDynamicUpdate() const;
    void setOggDynamicUpdate(bool value);

  signals:
    void profileNameChanged(const QString& oldName, const QString& newName);
    void statusChanged(bool newStatus);
    void connectionStatusChanged(int newConnectionStatus);

  public slots:
    void relayStatus(bool newStatus);
    void relayConnectionStatus(int newConnectionStatus);

  private:
    void adoptDefaultValues();
    bool loadValues(const QString& filename);

    bool setSecurePassword(const QString& login, const QString& password);
    QString getSecurePassword(const QString& login);

    void errorDialog(const QString& text, const QString& detailedError);

    bool m_secureCredentials;

    QString m_filename;

    QString m_profileName;
    bool m_enabled;

    QString m_host;
    int m_port;
    QString m_serverType;
    QString m_login;
    QString m_password;

    bool m_enableReconnect;
    double m_reconnectPeriod;
    bool m_limitReconnects;
    int m_maximumRetries;
    bool m_noDelayFirstReconnect;
    double m_reconnectFirstDelay;

    QString m_mountpoint;
    QString m_streamName;
    QString m_streamDesc;
    QString m_streamGenre;
    bool m_streamPublic;
    QString m_streamWebsite;
    QString m_streamIRC;
    QString m_streamAIM;
    QString m_streamICQ;

    QString m_format;
    int m_bitrate;
    int m_channels;

    bool m_enableMetadata;
    QString m_metadataCharset;
    QString m_customArtist;
    QString m_customTitle;
    QString m_metadataFormat;
    bool m_oggDynamicUpdate;

    QAtomicInt m_connectionStatus;
};

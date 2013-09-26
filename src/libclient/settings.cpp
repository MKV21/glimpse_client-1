#include "settings.h"

#include <QDebug>

class Settings::Private
{
public:
    GetConfigResponse config;
    QSettings settings;
};

Settings::Settings(QObject *parent)
: QObject(parent)
, d(new Private)
{
}

Settings::~Settings()
{
    d->settings.setValue("config", d->config.toVariant());
    delete d;
}

Settings::StorageType Settings::init()
{
    // Create new settings
    if ( deviceId().isNull() ) {
        setDeviceId( QUuid::createUuid() );

        qDebug() << "Created new settings for this device";

        return NewSettings;
    } else {
        d->config.fillFromVariant( qvariant_cast<QVariantMap>(d->settings.value("config")) );

        qDebug() << "Loaded existing settings for this device";
        return ExistingSettings;
    }
}

void Settings::setDeviceId(const QUuid &deviceId)
{
    if ( this->deviceId() != deviceId ) {
        settings.setValue("device-id", deviceId);
        emit deviceIdChanged(deviceId);
    }
}

QUuid Settings::deviceId() const
{
    return settings.value("device-id").toUuid();
}

void Settings::setUserId(const QString &userId)
{
    if ( this->userId() != userId ) {
        settings.setValue("user-id", userId);
        emit userIdChanged(userId);
    }
}

QString Settings::userId() const
{
    return settings.value("user-id").toString();
}

void Settings::setPassword(const QString &password)
{
    if ( this->password() != password ) {
        settings.setValue("password", password);
        emit passwordChanged(password);
    }
}

QString Settings::password() const
{
    return settings.value("password").toString();
}

void Settings::setSessionId(const QString &sessionId)
{
    if ( this->sessionId() != sessionId ) {
        settings.setValue("session-id", sessionId);
        emit sessionIdChanged(sessionId);
    }
}

QString Settings::sessionId() const
{
    return settings.value("session-id").toString();
}

GetConfigResponse *Settings::config() const
{
    return &d->config;
}

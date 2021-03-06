#include "client.h"
#include "settings.h"
#include "trafficbudgetmanager.h"
#include "network/networkmanager.h"
#include "log/logger.h"
#include "timing/calendartiming.h"
#include "timing/timer.h"

#include <QReadLocker>
#include <QWriteLocker>

LOGGER(TrafficBudgetManager);


class TrafficBudgetManager::Private : public QObject
{
    Q_OBJECT

public:
    Private(TrafficBudgetManager *q)
        : q(q)
        , resetTiming(new CalendarTiming(QDateTime(), QDateTime(), CalendarTiming::AllMonths, CalendarTiming::AllDaysOfWeek, QList<int>()<<1, QList<int>()<<0, QList<int>()<<1, QList<int>()<<0))
        , timer(resetTiming)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    }

    TrafficBudgetManager *q;
    Settings *settings;
    quint32 usedTraffic;
    quint32 usedMobileTraffic;
    QReadWriteLock lock;
    bool active;
    QSharedPointer<CalendarTiming> resetTiming;
    Timer timer;

public slots:
    void timeout();
};

void TrafficBudgetManager::Private::timeout()
{
    if (active)
    {
        LOG_INFO("Reset traffic budget for new month");
        LOG_INFO(QString("Traffic: Used %1 of %2 MB").arg(usedTraffic/(1024*1024)).arg(
                     settings->allowedTraffic()/(1024*1024)));
        LOG_INFO(QString("Traffic (mobile): Used %1 of %2 MB").arg(usedMobileTraffic/(1024*1024)).arg(
                     settings->allowedMobileTraffic()/(1024*1024)));
    }

    q->reset();
}

TrafficBudgetManager::TrafficBudgetManager(QObject *parent)
: QObject(parent)
, d(new Private(this))
{
}

TrafficBudgetManager::~TrafficBudgetManager()
{
    delete d;
}

void TrafficBudgetManager::init()
{
    d->settings = Client::instance()->settings();
    d->usedMobileTraffic = d->settings->usedMobileTraffic();
    d->usedTraffic = d->settings->usedTraffic();
    d->active = d->settings->trafficBudgetManagerActive();

    LOG_INFO(QString("Traffic budget manager is %1").arg(d->active ? "enabled" : "disabled"));

    if (d->active)
    {
        LOG_INFO(QString("Traffic: Used %1 of %2 MB").arg(d->usedTraffic/(1024*1024)).arg(
                     d->settings->allowedTraffic()/(1024*1024)));
        LOG_INFO(QString("Traffic (mobile): Used %1 of %2 MB").arg(d->usedMobileTraffic/(1024*1024)).arg(
                     d->settings->allowedMobileTraffic()/(1024*1024)));
    }

    d->timer.start();
}

void TrafficBudgetManager::saveTraffic()
{
    if (Client::instance()->networkManager()->onMobileConnection())
    {
        d->settings->setUsedMobileTraffic(d->usedMobileTraffic);
    }
    else
    {
        d->settings->setUsedTraffic(d->usedTraffic);
    }
}

bool TrafficBudgetManager::addUsedTraffic(quint32 traffic)
{
    QWriteLocker locker(&d->lock);

    quint32 availableTraffic = Client::instance()->networkManager()->onMobileConnection()
            ? (d->settings->allowedMobileTraffic() - d->usedMobileTraffic)
            : (d->settings->allowedTraffic() - d->usedTraffic);

    if (availableTraffic >= traffic)
    {
        if (Client::instance()->networkManager()->onMobileConnection())
        {
            d->usedMobileTraffic += traffic;
            saveTraffic();
            return true;
        }
        else
        {
            d->usedTraffic += traffic;
            saveTraffic();
            return true;
        }
    }

    return !d->active;
}

quint32 TrafficBudgetManager::usedTraffic() const
{
    QReadLocker locker(&d->lock);

    return Client::instance()->networkManager()->onMobileConnection() ? d->usedMobileTraffic : d->usedTraffic;
}

void TrafficBudgetManager::reset()
{
    d->usedMobileTraffic = 0;
    d->usedTraffic = 0;
}

#include "trafficbudgetmanager.moc"

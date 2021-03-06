#include "storagepaths.h"
#include "../log/logger.h"

#include <QAndroidJniObject>

LOGGER(StoragePaths);

class StoragePaths::Private
{
public:
    static bool initialized;
    static QDir scheduler;
    static QDir report;
    static QDir cache;
    static QDir log;
    static QDir crashDumps;
    static QDir result;
    static QDir task;
};

bool StoragePaths::Private::initialized = false;
QDir StoragePaths::Private::scheduler;
QDir StoragePaths::Private::report;
QDir StoragePaths::Private::cache;
QDir StoragePaths::Private::log;
QDir StoragePaths::Private::crashDumps;
QDir StoragePaths::Private::result;
QDir StoragePaths::Private::task;

StoragePaths::StoragePaths()
: d(NULL)
{
    if (StoragePaths::Private::initialized)
    {
        return;
    }

    QAndroidJniObject storageHelper("de/hsaugsburg/informatik/mplane/StorageHelper");

    StoragePaths::Private::scheduler = storageHelper.callObjectMethod<jstring>("getSchedulerDirectory").toString();
    StoragePaths::Private::report = storageHelper.callObjectMethod<jstring>("getReportDirectory").toString();
    StoragePaths::Private::cache = storageHelper.callObjectMethod<jstring>("getCacheDirectory").toString();
    StoragePaths::Private::log = storageHelper.callObjectMethod<jstring>("getLogDirectory").toString();
    StoragePaths::Private::crashDumps = storageHelper.callObjectMethod<jstring>("getCrashDumpDirectory").toString();
    StoragePaths::Private::result = storageHelper.callObjectMethod<jstring>("getResultDirectory").toString();
    StoragePaths::Private::task = storageHelper.callObjectMethod<jstring>("getTaskDirectory").toString();
    StoragePaths::Private::initialized = true;
}

StoragePaths::~StoragePaths()
{
}

QDir StoragePaths::schedulerDirectory() const
{
    return StoragePaths::Private::scheduler;
}

QDir StoragePaths::reportDirectory() const
{
    return StoragePaths::Private::report;
}

QDir StoragePaths::cacheDirectory() const
{
    return StoragePaths::Private::cache;
}

QDir StoragePaths::logDirectory() const
{
    return StoragePaths::Private::log;
}

QDir StoragePaths::crashDumpDirectory() const
{
    return StoragePaths::Private::crashDumps;
}

QDir StoragePaths::resultDirectory() const
{
    return StoragePaths::Private::result;
}

QDir StoragePaths::taskDirectory() const
{
    return StoragePaths::Private::task;
}

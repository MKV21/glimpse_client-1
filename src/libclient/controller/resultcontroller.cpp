#include "resultcontroller.h"
#include "../storage/storagepaths.h"
#include "../settings.h"

#include <QPointer>
#include <QDir>
#include <QDateTime>
#include <QSet>

class ResultController::Private : public QObject
{
    Q_OBJECT

public:
    Private(ResultController *q)
    : q(q)
    {

    }

    ResultController *q;

    QPointer<ResultScheduler> resultScheduler;
    QPointer<Scheduler> scheduler;
    QPointer<Settings> settings;

public slots:
    void rotate();
};

void ResultController::Private::rotate()
{
    // <task-id>_yyyy-MM-dd.json
    QRegExp regex("^(-?\\d+)_(\\d{4}-\\d{2}-\\d{2}).json$");
    QDir dir(StoragePaths().resultDirectory());
    QDate oldest = QDateTime::currentDateTime().addDays(-static_cast<qint64>(settings->backlog())).date();

    foreach (const QString &file, dir.entryList(QDir::Files))
    {
        if (regex.exactMatch(file))
        {
            if (QDateTime::fromString(regex.cap(2), "yyyy-MM-dd").date() < oldest)
            {
                dir.remove(file);
            }
        }
    }

    QSet<TaskId> resultIds;
    QSet<TaskId> taskIds;
    QSet<TaskId> schedulerIds;

    foreach (const QString &file, dir.entryList(QDir::Files))
    {
        if (regex.exactMatch(file))
        {
            resultIds << TaskId(regex.cap(1).toInt());
        }
    }

    dir = StoragePaths().schedulerDirectory();

    foreach (const QString &file, dir.entryList(QDir::Files))
    {
        schedulerIds << TaskId(file.toInt());
    }

    dir = StoragePaths().taskDirectory();

    foreach (const QString &file, dir.entryList(QDir::Files))
    {
        taskIds << TaskId(file.toInt());
    }

    foreach (const TaskId &id, taskIds.subtract(schedulerIds).subtract(resultIds))
    {
        // remove obsolete tasks
        dir.remove(QString::number(id.toInt()));
        scheduler->removeTask(id);
    }
}

ResultController::ResultController(QObject *parent)
: Controller(parent)
, d(new Private(this))
{
}

ResultController::~ResultController()
{
    d->rotate();
    delete d;
}

bool ResultController::init(ResultScheduler *resultScheduler, Scheduler *scheduler, Settings *settings)
{
    d->resultScheduler = resultScheduler;
    d->scheduler = scheduler;
    d->settings = settings;

    connect(d->resultScheduler, SIGNAL(resultAdded(const QVariantMap)), d, SLOT(rotate()));
    connect(d->resultScheduler, SIGNAL(resultModified(const QVariantMap)), d, SLOT(rotate()));

    return true;
}

QString ResultController::errorString() const
{
    return "";
}

ResultController::Status ResultController::status() const
{
    return Unknown;
}

#include "resultcontroller.moc"

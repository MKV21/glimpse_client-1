#ifndef REPORT_H
#define REPORT_H

#include "../serializable.h"
#include "../task/result.h"
#include "timing/timing.h"

class Report;
typedef QList<Report> ReportList;

class CLIENT_API Report : public Serializable
{
public:
    Report();
    Report(const Report &other);
    Report(const TaskId &taskId, const QDateTime &dateTime, const QString &appVersion, const ResultList &results, const QVariantMap &specification = QVariantMap(), const TimingPtr &timing = TimingPtr());
    ~Report();

    Report &operator=(const Report &rhs);
    bool operator ==(const Report &other) const;

    bool isNull() const;

    // Getters
    void setTaskId(const TaskId &id);
    TaskId taskId() const;

    void setDateTime(const QDateTime &dateTime);
    QDateTime dateTime() const;

    void setAppVersion(const QString &appVersion);
    QString appVersion() const;

    void setResults(const ResultList &results);
    ResultList results() const;

    void setSpecification(const QVariantMap &specification);
    QVariantMap specification() const;

    void setTiming(const TimingPtr &timing);
    TimingPtr timing() const;

    // Storage
    static Report fromVariant(const QVariant &variant);

    // Serializable interface
    QVariant toVariant() const;

private:
    QSharedDataPointer<class ReportData> d;
};

Q_DECLARE_METATYPE(Report)
Q_DECLARE_TYPEINFO(Report, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(ReportList)

#endif // REPORT_H

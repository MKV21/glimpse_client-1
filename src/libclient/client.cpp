#include "client.h"
#include "controller/taskcontroller.h"
#include "controller/reportcontroller.h"
#include "controller/logincontroller.h"
#include "controller/configcontroller.h"
#include "controller/crashcontroller.h"
#include "network/networkmanager.h"
#include "task/taskexecutor.h"
#include "scheduler/schedulerstorage.h"
#include "report/reportstorage.h"
#include "scheduler/scheduler.h"
#include "settings.h"
#include "log/logger.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QHostInfo>

#ifdef Q_OS_UNIX
#include <QSocketNotifier>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#endif // Q_OS_UNIX

#ifdef Q_OS_WIN
#include <Windows.h>
#include <stdio.h>
#endif // Q_OS_WIN

// TEST INCLUDES
#include "timing/immediatetiming.h"
#include "timing/timing.h"
#include "task/task.h"
#include "measurement/btc/btc_definition.h"
#include "measurement/http/httpdownload_definition.h"
#include "measurement/ping/ping_definition.h"
#include "measurement/dnslookup/dnslookup_definition.h"
#include "measurement/reverse_dnslookup/reverseDnslookup_definition.h"
#include "measurement/packettrains/packettrainsdefinition.h"
#include "measurement/udpping/udpping_definition.h"
#include "measurement/traceroute/traceroute_definition.h"

LOGGER(Client);

Q_GLOBAL_STATIC(Ntp, ntp)

class Client::Private : public QObject
{
    Q_OBJECT

public:
    Private(Client *q)
    : q(q)
    , status(Client::Unregistered)
    , networkAccessManager(new QNetworkAccessManager(q))
    , schedulerStorage(&scheduler)
    , reportStorage(&reportScheduler)
    {
        executor.setNetworkManager(&networkManager);
        scheduler.setExecutor(&executor);

        connect(&executor, SIGNAL(finished(TestDefinition, QStringList, Result)), this, SLOT(taskFinished(TestDefinition,
                                                                                                          QStringList,
                                                                                                          Result)));
        connect(&loginController, SIGNAL(finished()), this, SLOT(loginStatusChanged()));
    }

    Client *q;

    // Properties
    Client::Status status;
    QNetworkAccessManager *networkAccessManager;

    TaskExecutor executor;

    Scheduler scheduler;
    SchedulerStorage schedulerStorage;

    ReportScheduler reportScheduler;
    ReportStorage reportStorage;

    Settings settings;
    NetworkManager networkManager;

    TaskController taskController;
    ReportController reportController;
    LoginController loginController;
    ConfigController configController;
    CrashController crashController;

#ifdef Q_OS_UNIX
    static int sigintFd[2];
    static int sighupFd[2];
    static int sigtermFd[2];

    QSocketNotifier *snInt;
    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;

    // Unix signal handlers.
    static void intSignalHandler(int unused);
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
#endif // Q_OS_UNIX

    // Functions
    void setupUnixSignalHandlers();

#ifdef Q_OS_WIN
    static BOOL CtrlHandler(DWORD ctrlType);
#endif // Q_OS_WIN

public slots:
#ifdef Q_OS_UNIX
    void handleSigInt();
    void handleSigHup();
    void handleSigTerm();
#endif // Q_OS_UNIX
    void taskFinished(const TestDefinition &test, const QStringList &resultHeader, const Result &result);
    void loginStatusChanged();
};

#ifdef Q_OS_UNIX
int Client::Private::sigintFd[2];
int Client::Private::sighupFd[2];
int Client::Private::sigtermFd[2];

void Client::Private::intSignalHandler(int)
{
    char a = 1;
    ::write(sigintFd[0], &a, sizeof(a));
}

void Client::Private::hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void Client::Private::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}
#endif // Q_OS_UNIX

void Client::Private::setupUnixSignalHandlers()
{
#ifdef Q_OS_UNIX
    struct sigaction Int, hup, term;

    Int.sa_handler = Client::Private::intSignalHandler;
    sigemptyset(&Int.sa_mask);
    Int.sa_flags = 0;
    Int.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &Int, 0) > 0)
    {
        return;
    }

    hup.sa_handler = Client::Private::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0) > 0)
    {
        return;
    }

    term.sa_handler = Client::Private::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) > 0)
    {
        return;
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd))
    {
        qFatal("Couldn't create INT socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
    {
        qFatal("Couldn't create HUP socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
    {
        qFatal("Couldn't create TERM socketpair");
    }

    snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(snInt, SIGNAL(activated(int)), this, SLOT(handleSigInt()));

    snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));

    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
#elif defined(Q_OS_WIN)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Private::CtrlHandler, TRUE);
#endif
}

#ifdef Q_OS_WIN
BOOL Client::Private::CtrlHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        LOG_INFO("Close requested, quitting.");
        qApp->quit();
        return TRUE;

    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        LOG_INFO("System shutdown or user logout, quitting.");
        qApp->quit();
        return FALSE;

    default:
        return FALSE;
    }
}
#endif // Q_OS_WIN

#ifdef Q_OS_UNIX
void Client::Private::handleSigInt()
{
    snInt->setEnabled(false);
    char tmp;
    ::read(sigintFd[1], &tmp, sizeof(tmp));

    LOG_INFO("Interrupt requested, quitting.");
    qApp->quit();

    snInt->setEnabled(true);
}

void Client::Private::handleSigTerm()
{
    snTerm->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    LOG_INFO("Termination requested, quitting.");
    qApp->quit();

    snTerm->setEnabled(true);
}

void Client::Private::handleSigHup()
{
    snHup->setEnabled(false);
    char tmp;
    ::read(sighupFd[1], &tmp, sizeof(tmp));

    LOG_INFO("Hangup detected, quitting.");
    qApp->quit();

    snHup->setEnabled(true);
}
#endif // Q_OS_UNIX

void Client::Private::taskFinished(const TestDefinition &test, const QStringList &resultHeader, const Result &result)
{
    Report report = reportScheduler.reportByTaskId(test.id());

    ResultList results = report.isNull() ? ResultList() : report.results();
    results.append(result);

    if (report.isNull() || results.size() == 1)
    {
        report = Report(test.id(), QDateTime::currentDateTime(), Client::version(), resultHeader, results);
        reportScheduler.addReport(report);
    }
    else
    {
        report.setResults(results);
        reportScheduler.modifyReport(report);
    }
}

void Client::Private::loginStatusChanged()
{
    if (!settings.isPassive() && loginController.registeredDevice())
    {
        taskController.fetchTasks();
    }
}

Client::Client(QObject *parent)
: QObject(parent)
, d(new Private(this))
{
}

Client::~Client()
{
    d->schedulerStorage.storeData();
    d->reportStorage.storeData();
    delete d;
}

Client *Client::instance()
{
    static Client *ins = NULL;

    if (!ins)
    {
        ins = new Client();
    }

    return ins;
}

bool Client::init()
{
    qRegisterMetaType<TestDefinition>();
    qRegisterMetaType<Result>();

    // Get network time from ntp server
    QHostInfo hostInfo = QHostInfo::fromName("ptbtime1.ptb.de");

    if (!hostInfo.addresses().isEmpty())
    {
        QHostAddress ntpServer = hostInfo.addresses().first();
        ntp->sync(ntpServer);
    }
    else
    {
        LOG_WARNING("could not resolve ntp server");
    }

    d->setupUnixSignalHandlers();
    d->settings.init();

    // Initialize storages
    d->schedulerStorage.loadData();
    d->reportStorage.loadData();

    // Initialize controllers
    d->networkManager.init(&d->scheduler, &d->settings);
    d->configController.init(&d->networkManager, &d->settings);
    d->reportController.init(&d->reportScheduler, &d->settings);
    d->loginController.init(&d->networkManager, &d->settings);
    d->crashController.init(&d->networkManager, &d->settings);

    if (!d->settings.isPassive())
    {
        d->taskController.init(&d->networkManager, &d->scheduler, &d->settings);
    }

    return true;
}

bool Client::autoLogin()
{
    if (d->settings.hasLoginData())
    {
        // check if api key is still valid
        d->taskController.fetchTasks();

        return true;
    }

    return false;
}

void Client::btc(const QString &host)
{
    BulkTransportCapacityDefinition btcDef(host, 5106, 1024 * 1024, 10);
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(9, "btc_ma", timing,
                                  btcDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::http(const QString &url)
{
    HTTPDownloadDefinition httpDef(url, false);
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(3, "httpdownload", timing,
                                  httpDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::upnp()
{
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(5, "upnp", timing,
                                  QVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::ping(const QString &host, quint16 count, quint32 timeout, quint32 interval)
{
    PingDefinition pingDef(host.isNull() ? "measure-it.de" : host, count, timeout, interval);
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(1, "ping", timing,
                                  pingDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::dnslookup()
{
    DnslookupDefinition dnslookupDef("www.google.com", "8.8.8.8");
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(8, "dnslookup", timing,
                                  dnslookupDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::reverseDnslookup()
{
    ReverseDnslookupDefinition reverseDnslookupDef("8.8.8.8");
    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(9, "reverseDnslookup", timing,
                                  reverseDnslookupDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::packetTrains(QString host, quint16 port, quint16 packetSize, quint16 trainLength, quint8 iterations,
                          quint64 rateMin, quint64 rateMax, quint64 delay)
{
    PacketTrainsDefinition packetTrainsDef(host, port, packetSize, trainLength, iterations, rateMin, rateMax, delay);

    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(11, "packettrains_ma", timing,
                                  packetTrainsDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::udpPing(const QString &url, const quint32 &count, const quint32 &interval, const quint32 &receiveTimeout,
                     const int &ttl, const quint16 &destinationPort, const quint16 &sourcePort, const quint32 &payload,
                     const QAbstractSocket::SocketType &pingType)
{
    UdpPingDefinition udpPingDef(url, count, interval, receiveTimeout, ttl, destinationPort, sourcePort, payload, pingType);

    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(12, "udpping", timing,
                                  udpPingDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::traceroute(const QString &url,
                        const quint32 &count,
                        const quint32 &interval,
                        const quint32 &receiveTimeout,
                        const quint16 &destinationPort,
                        const quint16 &sourcePort,
                        const quint32 &payload,
                        const QAbstractSocket::SocketType pingType)
{
    TracerouteDefinition tracerouteDef(url, count, interval, receiveTimeout, destinationPort, sourcePort, payload, pingType);

    TimingPtr timing(new ImmediateTiming());
    TestDefinition testDefinition(13, "traceroute", timing,
                                  tracerouteDef.toVariant());
    d->scheduler.enqueue(testDefinition);
}

void Client::measureIt()
{
    // Ping
    d->scheduler.executeOnDemandTest(1);

    // BTC
    d->scheduler.executeOnDemandTest(2);

    // HTTP download
    d->scheduler.executeOnDemandTest(3);

    // PacketTrains
    d->scheduler.executeOnDemandTest(4);

    // UdpPing
    d->scheduler.executeOnDemandTest(5);

    // Traceroute
    d->scheduler.executeOnDemandTest(6);
}

void Client::setStatus(Client::Status status)
{
    if (d->status == status)
    {
        return;
    }

    d->status = status;
    emit statusChanged();
}

Client::Status Client::status() const
{
    return d->status;
}

QString Client::version()
{
    return QString("%1.%2.%3").arg(Client::versionMajor).arg(Client::versionMinor).arg(Client::versionPatch);
}

QNetworkAccessManager *Client::networkAccessManager() const
{
    return d->networkAccessManager;
}

Scheduler *Client::scheduler() const
{
    return &d->scheduler;
}

ReportScheduler *Client::reportScheduler() const
{
    return &d->reportScheduler;
}

NetworkManager *Client::networkManager() const
{
    return &d->networkManager;
}

TaskExecutor *Client::taskExecutor() const
{
    return &d->executor;
}

ConfigController *Client::configController() const
{
    return &d->configController;
}

LoginController *Client::loginController() const
{
    return &d->loginController;
}

ReportController *Client::reportController() const
{
    return &d->reportController;
}

TaskController *Client::taskController() const
{
    return &d->taskController;
}

CrashController *Client::crashController() const
{
    return &d->crashController;
}

Settings *Client::settings() const
{
    return &d->settings;
}

#include "client.moc"

#ifndef MPLANECONTROLLER_H
#define MPLANECONTROLLER_H

#include "controller.h"
#include "network/networkmanager.h"

class CLIENT_API MPlaneController : Controller
{
    Q_OBJECT
public:
    MPlaneController(QObject *parent = 0);
    ~MPlaneController();

    bool init(NetworkManager *networkManager,
              Scheduler *scheduler,
              Settings *settings);

    // Controller interface
    Status status() const;
    QString errorString() const;

    void sendCapabilities();

public slots:
    void fetchSpecifications();

signals:

protected:
    class Private;
    Private *d;
};

#endif // MPLANECONTROLLER_H

#include "btc_mp.h"
#include "../../log/logger.h"

#include <QDataStream>

LOGGER(BulkTransportCapacityMP)

BulkTransportCapacityMP::BulkTransportCapacityMP(QObject *parent)
: Measurement(parent)
{
    m_tcpSocket = 0;
    m_tcpServer = new QTcpServer(this);

    // Signal for errors
    connect(m_tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));

    // Signal for new clients
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newClientConnection()));
}

bool BulkTransportCapacityMP::start()
{
    // Start listening
    return m_tcpServer->listen(QHostAddress::Any, definition->port);
}

qint64 BulkTransportCapacityMP::sendResponse(quint64 bytes)
{
    // send data back
    QByteArray data;
    data.resize(bytes);
    data.fill('X'); // TODO do we need to randomize this to prevent compression on the path?
    return m_tcpSocket->write(data);
}

void BulkTransportCapacityMP::resetServer()
{
    m_tcpSocket = 0;
}

void BulkTransportCapacityMP::newClientConnection()
{
    LOG_INFO("New client connection");

    if (!m_tcpSocket)
    {
        m_tcpSocket = m_tcpServer->nextPendingConnection();

        connect(m_tcpSocket, SIGNAL(disconnected()), m_tcpSocket, SLOT(deleteLater()));
        connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveRequest()));
        connect(m_tcpSocket, SIGNAL(destroyed()), this, SLOT(resetServer()));
    }
    else
    {
        LOG_ERROR("There is already a client connected, abort");
        QTcpSocket* next = m_tcpServer->nextPendingConnection();
        next->abort();
        delete next;
    }
}

void BulkTransportCapacityMP::receiveRequest()
{
    LOG_INFO("New client request");

    // get bytes from message
    QDataStream in(m_tcpSocket);

    // abort if received data is not what we expected
    if (m_tcpSocket->bytesAvailable() < (int)sizeof(quint64))
    {
        LOG_ERROR("Data length is not what we expected");
        return;
    }

    quint64 bytes;
    in>>bytes;
    sendResponse(bytes);
}

void BulkTransportCapacityMP::handleError(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::RemoteHostClosedError)
        return;

    QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(sender());
    LOG_ERROR(QString("Socket Error: %1").arg(socket->errorString()));
}


Measurement::Status BulkTransportCapacityMP::status() const
{
    return Unknown;
}

bool BulkTransportCapacityMP::prepare(NetworkManager *networkManager, const MeasurementDefinitionPtr &measurementDefinition)
{
    definition = measurementDefinition.dynamicCast<BulkTransportCapacityDefinition>();
    return true;
}

bool BulkTransportCapacityMP::stop()
{
    return true;
}

ResultPtr BulkTransportCapacityMP::result() const
{
    return ResultPtr();
}

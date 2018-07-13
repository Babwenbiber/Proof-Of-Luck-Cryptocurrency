
#include "server.h"

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
//    qDebug() << Q_FUNC_INFO;
    listen(QHostAddress::Any);
}

void Server::incomingConnection(qintptr socketDescriptor)
{
//    qDebug() << Q_FUNC_INFO;
    Connection *connection = new Connection(this);
    connection->setSocketDescriptor(socketDescriptor);
    emit newConnection(connection);
}

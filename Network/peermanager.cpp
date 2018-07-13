#include "peermanager.h"

static const qint32 BroadcastInterval = 2000;
static const unsigned broadcastPort = 45000;

PeerManager::PeerManager(Client *client)
    : QObject(client)
{
//    qDebug() << Q_FUNC_INFO;
    this->client = client;

    QStringList envVariables;
    envVariables << "USERNAME" << "USER" << "USERDOMAIN"
                 << "HOSTNAME" << "DOMAINNAME";

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    foreach (QString string, envVariables) {
        if (environment.contains(string)) {
            address = environment.value(string).toUtf8();
            break;
        }
    }

    updateAddresses(); // TODO: change Addresses handling
    serverPort = 0;

    broadcastSocket.bind(QHostAddress::Any, broadcastPort, QUdpSocket::ShareAddress
                         | QUdpSocket::ReuseAddressHint);
    connect(&broadcastSocket, SIGNAL(readyRead()),
            this, SLOT(readBroadcastDatagram()));

    broadcastTimer.setInterval(BroadcastInterval);
    connect(&broadcastTimer, SIGNAL(timeout()),
            this, SLOT(sendBroadcastDatagram()));
}

void PeerManager::setServerPort(int port)
{
//    qDebug() << Q_FUNC_INFO;
    serverPort = port;
}

QByteArray PeerManager::getAddress() const
{
    return address;
}

QList<QHostAddress> PeerManager::getBroadcastAddresses() const
{
    return broadcastAddresses;
}

QList<QHostAddress> PeerManager::getIPAddresses() const
{
    return ipAddresses;
}

void PeerManager::startBroadcasting()
{
//    qDebug() << Q_FUNC_INFO;
    broadcastTimer.start();
}

bool PeerManager::isLocalHostAddress(const QHostAddress &address)
{
//    qDebug() << Q_FUNC_INFO << address;
    foreach (QHostAddress localAddress, ipAddresses) {
        if (address == localAddress)
            return true;
    }
    return false;
}

void PeerManager::sendBroadcastDatagram()
{
//    qDebug() << Q_FUNC_INFO;
    QByteArray datagram(address); // TODO: sign with own thingy
    datagram.append('@');
    datagram.append(QByteArray::number(serverPort));

    bool validBroadcastAddresses = true;
    foreach (QHostAddress address, broadcastAddresses) {
        if (broadcastSocket.writeDatagram(datagram, address,
                                          broadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if (!validBroadcastAddresses)
    {
        updateAddresses();
    }
}

void PeerManager::readBroadcastDatagram()
{
//    qDebug() << Q_FUNC_INFO;
    while (broadcastSocket.hasPendingDatagrams()) {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(broadcastSocket.pendingDatagramSize());
        if (broadcastSocket.readDatagram(datagram.data(), datagram.size(),
                                         &senderIp, &senderPort) == -1)
            continue;

        QList<QByteArray> list = datagram.split('@');
        if (list.size() != 2)
            continue;

        int senderServerPort = list.at(1).toInt();
        if (isLocalHostAddress(senderIp) && senderServerPort == serverPort)
        {
            continue;
        }
        if (!client->hasConnection(senderIp)) {
            Connection *connection = new Connection(this);
            emit newConnection(connection);
            connection->connectToHost(senderIp, senderServerPort);
        }
    }
}

void PeerManager::updateAddresses()
{
//    qDebug() << Q_FUNC_INFO;
// OLD: Network Discovery
//######################################
// DONT DELETE
//    broadcastAddresses.clear();
//    ipAddresses.clear();
//    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
//        foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
//            QHostAddress broadcastAddress = entry.broadcast();
//            if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost) {
//                broadcastAddresses << broadcastAddress;
//                ipAddresses << entry.ip();
//            }
//        }
//    }
//######################################

    broadcastAddresses.clear();
    ipAddresses.clear();
    // TODO: handle file contents, which could be created not properly
    QFile file("../config/addresses.csv");
    if (!file.open(QIODevice::ReadOnly))
    {
        cout << "Could not open file: addresses.csv" << endl;
        cout << "Create a file named \"addresses.csv\" in config folder. Create the folder config and a adresses.csv and then restart the app." << endl;
//        exit(1);
    }

    QTextStream in(&file);

    if(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList addresses = line.split(",");
        for(const auto& address : addresses)
        {
//            ipAddresses << QHostAddress(address);
            if (!address.isEmpty()) {
                broadcastAddresses << QHostAddress(address);
                cout << address.toStdString() << endl;
            }
        }
    }

}

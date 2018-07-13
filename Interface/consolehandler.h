#ifndef CONSOLEHANDLER_H
#define CONSOLEHANDLER_H


#include "../Network/client.h"
#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

class ConsoleHandler: public QObject
{
  Q_OBJECT
public:
    ConsoleHandler(void);
    QString strString;
    quint16 uiAttempts;

    Client client;
public slots:
    void OnKeyPressed(char);
private:

    QString seperateParametersFromConsoleInput(QString);
};

#endif // CONSOLEHANDLER_H

#ifndef INTER_CONSOLE_H
#define INTER_CONSOLE_H


#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <QRegularExpression>


class Console : public QThread
{
    Q_OBJECT
public:
    Console(void);
//    ~Console(void);
    void run();
signals:
    void KeyPressed(char);
};





#endif // INTER_CONSOLE_H

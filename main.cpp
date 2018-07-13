
#include "Interface/gui.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    ConsoleHandler *kbCheck = new ConsoleHandler();
    gui w;
    w.show();
//    kbCheck->kbUser->start();
    return app.exec();
}

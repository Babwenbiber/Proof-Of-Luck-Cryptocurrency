#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
//#include "Network/client.h"
#include "console.h"
#include "consolehandler.h"


namespace Ui {
class gui;

}

class gui : public QMainWindow
{
    Q_OBJECT

public:
    explicit gui(QWidget *parent = 0);
    ~gui();
    Console *keyBoardUser;
    ConsoleHandler  *consoleHandler;

private slots:
    void on_mineButton_clicked();
    void on_transactionButton_clicked();
    void on_balancesPublicKeysComboBox_currentIndexChanged(int index);


    void updateBlockchainTabAndCorrespondingElements();
    void updateReceiverComboBox();
//    void updateBalance();

    void updateLNLabel(double ln);
    void on_tabWidget_currentChanged(int index);

    void on_checkBox_stateChanged(int state);

    void on_detailedCheckBox_stateChanged(int arg1);


    void disableMineButton();
    void enableMineButton();
    void disableCheckBox();
    void enableCheckBox();

private:
    Ui::gui *ui;
};

#endif // GUI_H

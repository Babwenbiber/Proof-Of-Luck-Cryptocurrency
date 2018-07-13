#include "gui.h"
#include "ui_gui.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <qpushbutton.h>

gui::gui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::gui)
{
    ui->setupUi(this);

    consoleHandler = new ConsoleHandler();
    keyBoardUser = new Console();
    keyBoardUser->start();


    int userBalance = consoleHandler->client.executeBalancePrinting(getPublicBkey());

    ui->balanceLabel->setText(QString::number(userBalance));
    ui->LNLabel->setText(QString::fromStdString("0 for Block " + to_string(consoleHandler->client.executeGetLatestBlockIndex())));
    ui->amountSpinBox->setMaximum(userBalance);
    ui->balancesPublicKeysComboBox->setStyleSheet("QComboBox{ color: black; background-color: white; selection-color: black;} ");
    ui->receiverComboBox->setStyleSheet("QComboBox{ color: black; background-color: white; selection-color: black;} ");
    ui->receiverComboBox->setItemData( 1, QColor( Qt::black ), Qt::BackgroundRole );

    ui->blockchainLabel->setText(consoleHandler->client.executePrintChain(false));

    vector<string> publicKeysUsedInChain = consoleHandler->client.getAllParticipants();

    foreach (string publicKey, publicKeysUsedInChain)
    {
        ui->balancesPublicKeysComboBox->addItem(QString::fromStdString(publicKey));
    }


    connect(&consoleHandler->client, SIGNAL(stopMiningSignal()),this,SLOT(disableMineButton()));
    connect(&consoleHandler->client, SIGNAL(startMiningSignal()),this,SLOT(enableMineButton()));
    connect(&consoleHandler->client, SIGNAL(stopTransactionSignal()),this,SLOT(disableCheckBox()));
    connect(&consoleHandler->client, SIGNAL(startTransactionSignal()),this,SLOT(enableCheckBox()));
    connect(&consoleHandler->client, SIGNAL(updateLNSignal(double)),this,SLOT(updateLNLabel(double)));

    connect(&consoleHandler->client, SIGNAL(updateReceiverComboBox()),this,SLOT(updateReceiverComboBox()));
    connect(&consoleHandler->client, SIGNAL(printChainSignal()),this,SLOT(updateBlockchainTabAndCorrespondingElements()));
    connect (keyBoardUser, SIGNAL (KeyPressed(char)), consoleHandler, SLOT(OnKeyPressed(char)));


}

gui::~gui()
{
    keyBoardUser->exit();
    delete ui;
}


//


void gui::on_mineButton_clicked() //mine
{
    if (ui->mineButton->text() == "Start Mining") {
        consoleHandler->client.startMiningThread();
        ui->mineButton->setText("Stop Mining");
    }
    else
    {
        consoleHandler->client.stopMiningThread();
        ui->mineButton->setText("Cleaning up the last block creation.");
    }
}


void gui::on_transactionButton_clicked()
{
    if (ui->receiverComboBox->count() == 0)
    {
        return;
    }
    string pbkeyReceiver = consoleHandler->client.getPublicKeys().at(ui->receiverComboBox->currentIndex());
    int amountToSend = ui->amountSpinBox->value();
    QString params = QString::fromStdString(pbkeyReceiver) + "," + QString::number(amountToSend);
    consoleHandler->client.handleTransactionCommandWithParams(params);


    // refresh ui-elements
    int userBalance = ui->amountSpinBox->maximum();

//    ui->balanceLabel->setText(QString::number(userBalance - amountToSend)); //
    ui->amountSpinBox->setValue(0);
    ui->amountSpinBox->setMaximum(userBalance - amountToSend);

}

void gui::on_balancesPublicKeysComboBox_currentIndexChanged(int index)
{
    if (index >= 0)
    {
        ui->balanceLabel->setText(QString::number(consoleHandler->client.executeBalancePrinting(consoleHandler->client.getAllParticipants().at(index))));
    }
}

void gui::updateReceiverComboBox()
{
    ui->receiverComboBox->clear();
    foreach(string pbkey, consoleHandler->client.getPublicKeys())
    {
        ui->receiverComboBox->addItem(QString::fromStdString(pbkey));
    }
}

void gui::updateBlockchainTabAndCorrespondingElements()
{
    bool detailed = ui->detailedCheckBox->isChecked();
    ui->blockchainLabel->setText(consoleHandler->client.executePrintChain(detailed));
//    ui->blockchainLabel->setText(true);


    int balance = consoleHandler->client.executeBalancePrinting(getPublicBkey());

    ui->amountSpinBox->setMaximum(balance);
    ui->balanceLabel->setText(QString::number(balance));

    vector<string> publicKeysUsedInChain = consoleHandler->client.getAllParticipants();

    if (ui->balancesPublicKeysComboBox->count() != (signed int) publicKeysUsedInChain.size())
    {
        ui->balancesPublicKeysComboBox->clear();
        foreach (string publicKey, publicKeysUsedInChain)
        {
            ui->balancesPublicKeysComboBox->addItem(QString::fromStdString(publicKey));
        }
    }

}

void gui::updateLNLabel(double ln)
{
    ui->LNLabel->setText(QString::fromStdString(to_string(ln) + " for Block " + to_string(consoleHandler->client.executeGetLatestBlockIndex())));
}

void gui::on_tabWidget_currentChanged(int index)
{
    if ( index == 1)
    {
        vector<Transaction> myTransactions = consoleHandler->client.getMyTransactions();
        string newTransactionsLabelString = "";
        foreach (Transaction transaction, myTransactions)
        {
            newTransactionsLabelString.append(transaction.print());
        }

        ui->transactionLabel->setText(QString::fromStdString(newTransactionsLabelString));
    }
}

void gui::on_checkBox_stateChanged(int state)
{
    if (state == 2)
    {
        consoleHandler->client.addToList();
        consoleHandler->client.startTransactionThread();
    }
    else
    {
        consoleHandler->client.removeFromList();
        consoleHandler->client.stopTransactionThread();
    }
}

void gui::on_detailedCheckBox_stateChanged(int arg1)
{
    bool detailed = ui->detailedCheckBox->isChecked();
    ui->blockchainLabel->setText(consoleHandler->client.executePrintChain(detailed));
}

void gui::disableMineButton()
{
    ui->mineButton->setDisabled(true);
}

void gui::enableMineButton()
{
    ui->mineButton->setDisabled(false);
    ui->mineButton->setText("Start Mining");
}

void gui::disableCheckBox()
{
    ui->checkBox->setDisabled(true);
    ui->checkBox->setText("Leaving Test Mode");
}

void gui::enableCheckBox()
{
    ui->checkBox->setDisabled(false);
    ui->checkBox->setText("Test Mode");
}

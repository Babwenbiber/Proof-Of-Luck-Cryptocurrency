#include "consolehandler.h"

ConsoleHandler::ConsoleHandler(void)
    :client(shared_ptr<recursive_mutex> (new recursive_mutex),
            shared_ptr<recursive_mutex> (new recursive_mutex),
            shared_ptr<mutex> (new mutex),
            shared_ptr<vector<string>> (new vector<string>),
            shared_ptr<mutex> (new mutex),
            shared_ptr<vector<string>> (new vector<string>))
{
    strString = "";
    uiAttempts = 3;

    cout << endl << "'print balance', 'print balance <key>' or 'quit'" << endl << endl;
}

void ConsoleHandler::OnKeyPressed(char cCurrent)
{
  if (cCurrent == 10 ) // enter
  {
    if (strString == "quit")
    {
        client.closeDb();
        QCoreApplication::exit(0);
    }
    else if (strString == "mine")
    {
        client.startMiningThread();
        strString.clear();
    }
    else if (strString == "stop mine")
    {
        client.stopMiningThread();
        strString.clear();
    }
    else if (strString == "verify Blockchain")
    {
        if (client.executeverifyBlockchain())
        {
            cout << "the Blockchain is valid" << endl;
        } else {
            cout << "the Blockchain is invalid" << endl;
        }
        strString.clear();

    }
    else if (strString == "print mempool")
    {
        client.executePrintMempool();
        strString.clear();

    }
    else if (strString == "print keys")
    {
        client.executePrintKeys();
        strString.clear();

    }
    else if (strString.contains("start Test",Qt::CaseInsensitive))
    {
        client.startTransactionThread();
        strString.clear();

    }
    else if (strString.contains("stop Test",Qt::CaseInsensitive))
    {
        client.stopTransactionThread();
        strString.clear();

    }
    else if (strString == "print balance")
    {
        client.executeBalancePrinting(getPublicBkey());
        strString.clear();
    }
    else if (strString.contains("print", Qt::CaseInsensitive)
             && strString.contains("balance", Qt::CaseInsensitive))
    {
        QStringList paramsList = strString.split(" ");
        string pk = getPublicBkey();  //default public key is your own
        if (paramsList.length() > 3)
        {
            cout << "Error: command \"print balance <key>\" needs 3 params to function" << endl;
            cout << "Use print balance without a key for your own balance OR" << endl;
            cout << "Use this sample: print balance ch723fh8f239h2398hf23f" << endl;
            return;
        }
        else if (paramsList.length() == 3)//use given public key
        {
            pk = paramsList.at(2).toUtf8().constData();
        }
        client.executeBalancePrinting(pk);
        strString.clear();
    }
    else if (strString.contains("print", Qt::CaseInsensitive)
             && strString.contains("history", Qt::CaseInsensitive))
    {
        QStringList paramsList = strString.split(" ");
        string pk = getPublicBkey();  //default public key is your own
        if (paramsList.length() > 3)
        {
            cout << "Error: command \"print history <key>\" needs 3 params to function" << endl;
            cout << "Use print balance without a key for your own balance OR" << endl;
            cout << "Use this sample: print history ch723fh8f239h2398hf23f" << endl;
            return;
        }
        else if (paramsList.length() == 3)//use given public key
        {
            pk = paramsList.at(2).toUtf8().constData();
        }
        client.executeHistoryPrinting();
        strString.clear();
    }
    else if (strString.contains("transaction", Qt::CaseInsensitive)) //TODO: define command
    {
        QString params = seperateParametersFromConsoleInput(strString);
        QStringList paramsList = params.split(",");
        if (paramsList.length() != 2)
        {
            cout << "Error: command \"transaction\" needs 2 params to function" << endl;
            cout << "Use this sample: transaction(recipient,amount)" << endl;
            return;
        }

        client.handleTransactionCommandWithParams(params);
        strString.clear();
    }
    else
    {
        strString.clear();
    }
  }
  else {
      strString.append(cCurrent);
  }
  return;
}

QString ConsoleHandler::seperateParametersFromConsoleInput(QString input) {
    QRegularExpression regex("\\((.*?)\\)");
    QRegularExpressionMatch match = regex.match(input, 0, QRegularExpression::PartialPreferFirstMatch);
    QString returnString = match.captured(0);
    returnString.replace(QString("("), QString(""));
    returnString.replace(QString(")"), QString(""));
    return returnString;
}

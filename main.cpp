#include <QCoreApplication>
#include <iostream>
#include "server.h"
#include "database.h"
#include <QDebug>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server myServer;

    /*Database db(QString("24.180.56.55"), 3066, QString("chathack"), QString("root"), QString(""));
    QList<QString> results = db.getChannelMembers(QString("hack"));
    QList<QString>::iterator i;
    for (i = results.begin(); i != results.end(); ++i)
    {
        qDebug() << *i;
    }*/

    return a.exec();
}

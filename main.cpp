#include <QtCore/QCoreApplication>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QObject>
#include <QThread>


#include "stdinThread.h"
#include "NetGet.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NetGet ng;
    
    QThread* workerThread1 = new QThread;
    stdinThread stdinthread;

    QObject::connect(workerThread1, SIGNAL(started()), &stdinthread, SLOT(doWork()));
    QObject::connect(workerThread1, SIGNAL(finished()), &stdinthread, SLOT(deleteLater()));

    stdinthread.moveToThread(workerThread1);

    workerThread1->start();

    return a.exec();
}

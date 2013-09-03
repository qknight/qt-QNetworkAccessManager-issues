#include "NetGet.h"

#include <QFile>
#include <QTextStream>
#include <QString>

NetGet::NetGet(QObject* parent) : QObject(parent) {
    qDebug() << __FUNCTION__ << "about to do lookupHost";
    info = QHostInfo::lookupHost("www.qt-project.org", this, SLOT(printResults(QHostInfo)));

    qDebug() << "lookupHost ID = " << info;

    qDebug() << __FUNCTION__ << "now invoking quit() at QCoreApplication::instance()";

    //FIXME uncommenting the line below will have an immediate effect
//     QHostInfo::abortHostLookup ( info );
}


NetGet::~NetGet() {
    qDebug() << "canceling lookupHost ID = " << info;

    //FIXME this seems to have no effect on my system
    QHostInfo::abortHostLookup ( info );
    qDebug() << __FUNCTION__ ;
}


void NetGet::printResults(QHostInfo h) {
    qDebug() << "--------------------------------------------\n" << __FUNCTION__ << h.addresses() << "\n--------------------------------------------" ;
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}


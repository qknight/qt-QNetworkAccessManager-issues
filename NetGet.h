#ifndef NETGET__HH
#define NETGET__HH

#include <QtCore/QCoreApplication>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QHostInfo>


class NetGet : public QObject {
    Q_OBJECT
public:
    NetGet(QObject* parent = 0);
    ~NetGet();
private:
    int info;
private slots:
    void printResults(QHostInfo h);
    void myQuit();
};

#endif

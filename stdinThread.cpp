#include "stdinThread.h"
#include <QCoreApplication>


void stdinThread::doWork() {

    QString s;
    QFile in;

    qDebug() << \
"------------------------------------------------------------------------------------------\n \
 type the letter 'q' and hit RETURN to run quit(\n\
------------------------------------------------------------------------------------------\n";

    in.open(stdin, QIODevice::ReadOnly);
    QTextStream ts(&in);

    while (true) {
        ts >> s;
        qDebug() << "you typed: " << s;

        if (s == "q") {
            emit myquit();
            break;
        }
    }
    qDebug() << "input handling exited, program should quit in less than a second!";
}

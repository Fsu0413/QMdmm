// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"

#include <QMdmmGlobal>
#include <QMdmmServer>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(u"Fsu0413.me"_s);
    QCoreApplication::setApplicationName(u"QMdmmServer"_s);
    QCoreApplication::setApplicationVersion(QMdmmCore::Global::version().toString());

    QString logDirectory = u"" QMDMM_RUNTIME_DATA_PREFIX "/log"_s;

    if (QDir().mkpath(logDirectory)) {
        QString logFilePath = QDir(logDirectory).absoluteFilePath(u"QMdmmServer-"_s + QString::number(QDateTime::currentMSecsSinceEpoch()));
        QFile *logFile = new QFile(logFilePath);

        if (logFile->open(QIODevice::WriteOnly)) {
            logFile->setParent(&a);
            QMdmmCore::qMdmmDebugSetDevice(logFile);
        } else {
            delete logFile;
            qCritical("Unable to create log file %s .", qPrintable(logFilePath));
        }
    } else {
        qCritical("Unable to create log directory %s .", qPrintable(logDirectory));
    }

    Config config;

    QMdmmNetworking::Server server(config.serverConfiguration(), config.logicConfiguration());

    QObject::connect(&server, &QMdmmNetworking::Server::listenError, &a, [&](const QString &transportName, const QString &errorString) {
        qCritical("Unable to listen on %s: %s", qPrintable(transportName), qPrintable(errorString));
    });

    bool listen = server.listen();
    if (!listen)
        qFatal("Unable to listen, exiting.");

    return QCoreApplication::exec();
}

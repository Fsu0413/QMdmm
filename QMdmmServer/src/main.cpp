// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"

#include <QMdmmGlobal>
#include <QMdmmServer>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Fsu0413.me"));
    QCoreApplication::setApplicationName(QStringLiteral("QMdmmServer"));
    QCoreApplication::setApplicationVersion(QMdmmCore::Global::version().toString());

    QString logDirectory = QStringLiteral(QMDMM_RUNTIME_DATA_PREFIX "/log");

    if (QDir().mkpath(logDirectory)) {
        QString logFilePath = QDir(logDirectory).absoluteFilePath(QStringLiteral("QMdmmServer-") + QString::number(QDateTime::currentMSecsSinceEpoch()));
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

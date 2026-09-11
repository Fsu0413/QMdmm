// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"
#include "config.h"

#include <QMdmmClient>
#include <QMdmmGlobal>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(u"Fsu0413.me"_s);
    QCoreApplication::setApplicationName(u"QMdmmBot"_s);
    QCoreApplication::setApplicationVersion(QMdmmCore::Global::version().toString());

    QString logDirectory = u"" QMDMM_RUNTIME_DATA_PREFIX "/log"_s;

    if (QDir().mkpath(logDirectory)) {
        QString logFilePath = QDir(logDirectory).absoluteFilePath(u"QMdmmBot-"_s + QString::number(QDateTime::currentMSecsSinceEpoch()));
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

    QMdmmNetworking::ClientConfiguration conf;
    conf.setScreenName(config.name());

    QMdmmNetworking::Client client(conf);

    Bot *bot = Bot::createBot(config.playingStyle(), &client);

    Q_UNUSED(bot);

    // TODO: implement

    client.connectToHost(config.host(), QMdmmCore::Data::StateOnlineBot);

    return QCoreApplication::exec();
}

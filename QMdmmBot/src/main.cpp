// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QMdmmGlobal>
#include <QMdmmServer>

#include <QCoreApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Fsu0413.me"));
    QCoreApplication::setApplicationName(QStringLiteral("QMdmmBot"));
    QCoreApplication::setApplicationVersion(QMdmmCore::Global::version().toString());

    QString logDirectory = QStringLiteral(QMDMM_RUNTIME_DATA_PREFIX "/log");

    if (!QDir().mkpath(logDirectory))
        qFatal("Unable to create log directory %s, exiting.", qPrintable(logDirectory));

    QString logFilePath = QDir(logDirectory).absoluteFilePath(QStringLiteral("QMdmmBot-") + QString::number(QDateTime::currentMSecsSinceEpoch()));
    QFile logFile(logFilePath);

    if (!logFile.open(QIODevice::WriteOnly))
        qFatal("Unable to create log file %s, exiting.", qPrintable(logFilePath));

    QMdmmCore::qMdmmDebugSetDevice(&logFile);

    // TODO: implement

    return QCoreApplication::exec();
}

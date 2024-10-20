// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"

#include <QMdmmGlobal>
#include <QMdmmServer>

#include <QCoreApplication>
#include <QScopedPointer>

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> a(new QCoreApplication(argc, argv));
    QCoreApplication::setOrganizationName(QStringLiteral("Fsu0413.me"));
    QCoreApplication::setApplicationName(QStringLiteral("QMdmmServer"));
    QCoreApplication::setApplicationVersion(QMdmmGlobal::version().toString());

    Config config;

    QMdmmServer server(config.serverConfiguration(), config.logicConfiguration());
    server.listen();

    return QCoreApplication::exec();
}

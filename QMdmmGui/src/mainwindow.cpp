// SPDX-License-Identifier: AGPL-3.0-or-later

#include "mainwindow.h"

#include "gameclient.h"

#include <QMdmmPlayer>

#include <QQuickWidget>
#include <QtQml>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QObject::tr("QMdmm"));

    // Register the core data enums and the Player type so the QML layer can
    // read properties and use action / upgrade constants.
    qmlRegisterUncreatableMetaObject(QMdmmCore::Data::staticMetaObject, "QMdmm.Core", 1, 0, "Data", QStringLiteral("Access to enums only"));
    qmlRegisterUncreatableType<QMdmmCore::Player>("QMdmm.Core", 1, 0, "Player", QStringLiteral("Player is created by the engine"));

    QMdmmGameClient *game = new QMdmmGameClient(this);

    QQuickWidget *qw = new QQuickWidget(QStringLiteral("qrc:///qt/qml/QMdmm/Gui/qml/main.qml"), this);

    qw->setResizeMode(QQuickWidget::SizeViewToRootObject);
    qw->rootContext()->setContextProperty(QStringLiteral("game"), (QObject *)game);
    qw->rootContext()->setContextProperty(QStringLiteral("MainWindowInstance"), (QObject *)this);

    setCentralWidget(qw);
}

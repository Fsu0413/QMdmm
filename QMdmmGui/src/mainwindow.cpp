// SPDX-License-Identifier: AGPL-3.0-or-later

#include "mainwindow.h"

#include <QQuickWidget>
#include <QtQml>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QObject::tr("QMdmm"));

    QQuickWidget *qw = new QQuickWidget(QStringLiteral("qrc:///qt/qml/QMdmm/Gui/qml/main.qml"), this);

    qw->setResizeMode(QQuickWidget::SizeViewToRootObject);
    qw->rootContext()->setContextProperty(QStringLiteral("cppif"), (QObject *)(&cppif));
    qw->rootContext()->setContextProperty(QStringLiteral("MainWindowInstance"), (QObject *)this);

    setCentralWidget(qw);
}

// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmgui.h"

#include "cppif.h"

#include <QFont>
#include <QGuiApplication>
#include <QPointer>
#include <QQuickView>
#include <QtQml>

QPointer<QQuickView> GlobalViewInstance = nullptr;

int main(int argc, char *argv[])
{
    [[maybe_unused]] QGuiApplication a(argc, argv);

    // Make font suitable for displaying
    QFont font = QGuiApplication::font();
    font.setPixelSize(50);
    QGuiApplication::setFont(font);

    QQuickView v;
    GlobalViewInstance = &v;

    QMdmmCppIf cppif;

    v.setTitle(QObject::tr("QMdmm"));
    v.setResizeMode(QQuickView::SizeViewToRootObject);
    v.rootContext()->setContextProperty(QStringLiteral("cppif"), (QObject *)(&cppif));
    v.setSource(QStringLiteral("qrc:///qt/qml/QMdmm/Gui/qml/main.qml"));

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    v.showMaximized();
#else
    v.show();
#endif

    // NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape)
    return QGuiApplication::exec();
}

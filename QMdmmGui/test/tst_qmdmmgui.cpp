// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QtQuickTest/quicktest.h>

#include <QtQml>

#include <QMdmmData>
#include <QMdmmPlayer>

#include "gameclient.h"

// Registers the GUI bridge type and the core data types with the QML engine so
// the QML test cases can instantiate QMdmmGameClient and inspect the Player
// objects / enum values it exposes. Mirrors the registration done by MainWindow
// (which is not linked into this test).
class QMdmmGuiTestSetup : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    QMdmmGuiTestSetup()
    {
        qmlRegisterUncreatableMetaObject(QMdmmCore::Data::staticMetaObject, "QMdmm.Core", 1, 0, "Data", QStringLiteral("Access to enums only"));
        qmlRegisterUncreatableType<QMdmmCore::Player>("QMdmm.Core", 1, 0, "Player", QStringLiteral("Player is created by the engine"));
        qmlRegisterType<QMdmmGameClient>("QMdmm.Gui", 1, 0, "GameClient");
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qmdmmgui, QMdmmGuiTestSetup)

#include "tst_qmdmmgui.moc"

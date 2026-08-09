#include "test.h"

#include <QGlobalStatic>
#include <QTest>

#include <memory>

namespace {
Q_GLOBAL_STATIC(QList<const QMetaObject *>, testObjects)
}

void registerTestObjectImpl(const QMetaObject *metaObject)
{
    testObjects->push_back(metaObject);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int ret = 0;

    foreach (const QMetaObject *ob, *testObjects) {
        std::unique_ptr<QObject> toBeTested {ob->newInstance()};
        if (toBeTested == nullptr)
            qFatal("%s can't be created", ob->className());

        QStringList args {
            QString::fromLatin1(ob->className()),
            QStringLiteral("-o"),
            QStringLiteral("-,txt"),
            QStringLiteral("-o"),
            QStringLiteral("%1%2.xml,junitxml").arg(QString::fromLatin1(ob->className()), QString::number(QT_VERSION_MAJOR)),
        };

        // This function does not seem to support multi-threading
        // Packeting the logic into a lambda and call it in a std::async crashes the program
        ret += QTest::qExec(toBeTested.get(), args);
    }

    return ret;
}

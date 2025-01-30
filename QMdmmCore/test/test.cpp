#include "test.h"

#include <QGlobalStatic>
#include <QTest>

Q_GLOBAL_STATIC(QList<const QMetaObject *>, testObjects)

void registerTestObjectImpl(const QMetaObject *metaObject)
{
    testObjects->push_back(metaObject);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int ret = 0;

    foreach (const QMetaObject *ob, *testObjects) {
        QObject *toBeTested = ob->newInstance();
        if (toBeTested == nullptr)
            qFatal("%s can't be created", ob->className());

        QStringList args {
            QString::fromLatin1(ob->className()),
            QStringLiteral("-o"),
            QStringLiteral("-,txt"),
            QStringLiteral("-o"),
            QStringLiteral("%1%2.xml,junitxml").arg(QString::fromLatin1(ob->className()), QString::number(QT_VERSION_MAJOR)),
        };

        ret += QTest::qExec(toBeTested, args);
    }

    return ret;
}

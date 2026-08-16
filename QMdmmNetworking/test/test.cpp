#include "test.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTest>

#include <memory>

const QMetaObject *registerTestObjectImpl(const QMetaObject *metaObject)
{
    static const QMetaObject *ob = metaObject;

    if (metaObject == nullptr)
        return ob;

    if (ob == nullptr)
        ob = metaObject;
    else if (ob != metaObject)
        qFatal("registerTestObjectImpl: multiple meta objects registered");

    return ob;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    static const QMetaObject *ob = registerTestObjectImpl();

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

    return QTest::qExec(toBeTested.get(), args);
}

#include "test.h"

#include <QMdmmCoreGlobal>

#include <QCoreApplication>
#include <QDebug>
#include <QTest>

#include <memory>

// NOLINTBEGIN
// Exempt from clang-tidy by policy; see AGENTS.md.

using namespace Qt::StringLiterals;

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
        QString::fromLatin1(ob->className()), u"-o"_s, u"-,txt"_s, u"-o"_s, u"%1%2.xml,junitxml"_s.arg(QString::fromLatin1(ob->className()), QString::number(QT_VERSION_MAJOR)),
    };

    return QTest::qExec(toBeTested.get(), args);
}

// NOLINTEND

#include "test.h"

#include <QMdmmCore/QMdmmRoom>

#include <QTest>

class tst_QMdmmRoom : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmRoom() = default;

private slots:
    // TBD
};

namespace {
RegisterTestObject<tst_QMdmmRoom> _;
}
#include "tst_qmdmmroom.moc"

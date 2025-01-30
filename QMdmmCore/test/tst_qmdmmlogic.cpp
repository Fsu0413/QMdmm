#include "test.h"

#include <QMdmmCore/QMdmmLogic>
#include <QMdmmCore/QMdmmLogicConfiguration>

#include <QTest>

class tst_QMdmmLogic : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmLogic() = default;

private:
    static QMdmmLogicConfiguration defaultConf;

private slots:
    // TBD
};

QMdmmLogicConfiguration tst_QMdmmLogic::defaultConf = QMdmmLogicConfiguration::defaults();

namespace {
RegisterTestObject<tst_QMdmmLogic> _;
}
#include "tst_qmdmmlogic.moc"

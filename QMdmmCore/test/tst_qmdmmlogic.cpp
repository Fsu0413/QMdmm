
#include <QMdmmCore/QMdmmLogic>
#include <QMdmmCore/QMdmmLogicConfiguration>

#include <QTest>

class tst_QMdmmLogic : public QObject
{
    Q_OBJECT

private:
    static QMdmmLogicConfiguration defaultConf;

private slots:
    // TBD
};

QMdmmLogicConfiguration tst_QMdmmLogic::defaultConf = QMdmmLogicConfiguration::defaults();

QTEST_GUILESS_MAIN(tst_QMdmmLogic)
#include "tst_qmdmmlogic.moc"

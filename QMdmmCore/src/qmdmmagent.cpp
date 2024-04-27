#include "qmdmmagent.h"
#include <QIODevice>

struct QMdmmAgentPrivate
{
    QString screenName;
};

QMdmmAgent::QMdmmAgent(QString name, QObject *parent)
    : QObject(parent)
    , d(new QMdmmAgentPrivate)
{
    setObjectName(name);
}

QMdmmAgent::~QMdmmAgent()
{
    delete d;
}

void QMdmmAgent::setStream(QIODevice *stream)
{
    (void)stream;
}

QString QMdmmAgent::screenName() const
{
    return d->screenName;
}

void QMdmmAgent::setScreenName(const QString &name)
{
    d->screenName = name;
}

void QMdmmAgent::streamReadyRead()
{
}

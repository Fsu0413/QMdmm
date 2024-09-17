// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmagent.h"

#include <QMdmmCore/QMdmmLogic>

#include <QPointer>

struct QMdmmLogicRunnerPrivate
{
    QPointer<QMdmmLogic> logic;
};

QMdmmLogicRunner::QMdmmLogicRunner(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QThread(parent)
    , d(new QMdmmLogicRunnerPrivate)
{
    d->logic = new QMdmmLogic(logicConfiguration);
    d->logic->moveToThread(this);
    // connect(this, &QMdmmLogicRunner::started, d->logic, &QMdmmLogic::run);
    connect(this, &QMdmmLogicRunner::finished, d->logic, &QMdmmLogic::deleteLater);
}

QMdmmLogicRunner::~QMdmmLogicRunner()
{
    delete d;
}

bool QMdmmLogicRunner::registerAgent(QMdmmAgent *agent)
{
    return false;
}

bool QMdmmLogicRunner::deregisterAgent(QMdmmAgent *agent)
{
    return false;
}

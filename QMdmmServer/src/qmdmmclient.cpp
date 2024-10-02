// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

QMdmmClientPrivate::QMdmmClientPrivate(QMdmmClient *p)
    : p(p)
    , socket(nullptr)
{
}

QMdmmClient::QMdmmClient(QObject *parent)
    : QObject(parent)
    , d(new QMdmmClientPrivate(this))
{
}

QMdmmClient::~QMdmmClient()
{
    delete d;
}

// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

Bot::Bot(QMdmmNetworking::Client *parent)
    : QObject(parent)
{
    // TODO: need to connect every needed signals from Client->agent here
    // expecially request and notify game process

    // The slot may be virtual for responding the notification, but...
    // maybe QueuedConnection and separate thread are needed?.. I don't think it is necessary except for RL bot
}

// There are no other suitable pure virtual functions so...
// let's make the dtor pure virtual so that it must be inherited
Bot::~Bot() = default;

QMdmmNetworking::Client *Bot::client()
{
    // TODO: nolintnextline comment
    return static_cast<QMdmmNetworking::Client *>(parent());
}

const QMdmmNetworking::Client *Bot::client() const
{
    // TODO: nolintnextline comment
    return static_cast<const QMdmmNetworking::Client *>(parent());
}

Bot *Bot::createBot(const QString &style, QMdmmNetworking::Client *parent)
{
    // TODO: derived class
    Q_UNUSED(style);
    Q_UNUSED(parent);
    return nullptr;
}

bool Bot::styleExist(const QString &style)
{
    // TODO: derived class
    Q_UNUSED(style);
    return false;
}

// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMBOT_CONFIG_H
#define QMDMMBOT_CONFIG_H

#include <QMdmmClient>
#include <QMdmmRoom>
#include <QMdmmSettings>

#include <QCommandLineParser>
#include <QSettings>

class Config
{
public:
    Config();

    [[nodiscard]] QString host() const
    {
        return host_;
    }

    [[nodiscard]] QString name() const
    {
        return name_;
    }

    [[nodiscard]] QString playingStyle() const
    {
        return playingStyle_;
    }

private:
    void read_(QCommandLineParser *parser);

    QString host_;
    QString name_;
    QString playingStyle_;
};

#endif

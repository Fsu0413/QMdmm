// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_CONFIG_H
#define QMDMMSERVER_CONFIG_H

#include <QMdmmRoom>
#include <QMdmmServer>
#include <QMdmmSettings>

#include <QCommandLineParser>
#include <QSettings>

class Config
{
public:
    Config();

    [[nodiscard]] const QMdmmNetworking::ServerConfiguration &serverConfiguration() const;
    [[nodiscard]] const QMdmmCore::LogicConfiguration &logicConfiguration() const;

private:
    QMdmmNetworking::ServerConfiguration serverConfiguration_;
    QMdmmCore::LogicConfiguration logicConfiguration_;

    void read_(QMdmmCore::Settings *setting, QCommandLineParser *parser);
    int save_(QMdmmCore::Settings *setting, QMdmmCore::Settings::Instance toSave);
    void show_();
};

#endif

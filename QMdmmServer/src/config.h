// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef CONFIG_H
#define CONFIG_H

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
    [[noreturn]] void save_(QMdmmCore::Settings *setting, QMdmmCore::Settings::Instance toSave);
    [[noreturn]] void show_();
};

#endif

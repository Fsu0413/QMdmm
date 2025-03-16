// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef CONFIG_H
#define CONFIG_H

#include <QMdmmRoom>
#include <QMdmmServer>
#include <QMdmmSettings>

#include <QCommandLineParser>
#include <QSettings>

using QMdmmSettings = QMdmmCore::QMdmmSettings;

class Config
{
public:
    Config();

    [[nodiscard]] const QMdmmServerConfiguration &serverConfiguration() const;
    [[nodiscard]] const QMdmmLogicConfiguration &logicConfiguration() const;

private:
    QMdmmServerConfiguration serverConfiguration_;
    QMdmmLogicConfiguration logicConfiguration_;

    void read_(QMdmmSettings *setting, QCommandLineParser *parser);
    [[noreturn]] void save_(QMdmmSettings *setting, QMdmmSettings::Instance toSave);
    [[noreturn]] void show_();
};

#endif

// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef CONFIG_H
#define CONFIG_H

#include <QMdmmRoom>
#include <QMdmmServer>

class QSettings;
class QCommandLineParser;

class Config
{
public:
    Config();

    [[nodiscard]] const QMdmmServerConfiguration &serverConfiguration() const;
    [[nodiscard]] const QMdmmLogicConfiguration &logicConfiguration() const;

private:
    QMdmmServerConfiguration serverConfiguration_;
    QMdmmLogicConfiguration logicConfiguration_;

    void read_(QSettings *systemConfig, QSettings *userConfig, QCommandLineParser *parser);
    [[noreturn]] void save_(QSettings *config);
    [[noreturn]] void show_();
};

#endif

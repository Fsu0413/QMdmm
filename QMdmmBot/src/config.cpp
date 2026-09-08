// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"
#include "bot.h"

#include <QMdmmSettings>

#include <QCommandLineParser>

#include <iostream>

static const QString helpText = QStringLiteral(R"help(Usage: QMdmmBot [options]

Options:
  -h, --help                         Show this help text and exit.
  -v, --version                      Show version information and exit.

Connection:
  -l, --host <host url>              Server address to connect to (required).
  -n, --name <screen name>           Screen name shown to other players (default: empty).

Bot:
  -s, --playing-style <style>        Playing style of this bot (default: knifePreferred).
                                     One of:
                                       knifePreferred      prefer the knife
                                       horsePreferred      prefer the horse
                                       rl                  reinforcement learning (not implemented)
)help");

namespace {

[[noreturn]] void configErrorImpl(const QString &message)
{
    std::cerr << qPrintable(message) << '\n' << std::flush;
    qWarning().noquote() << message;

    std::exit(3);
}

inline void configErrorArgs(QString &message)
{
    Q_UNUSED(message);
}

template<typename T, typename... Rest>
void configErrorArgs(QString &message, T &&arg, Rest &&...rest)
{
    message = message.arg(std::forward<T>(arg));
    configErrorArgs(message, std::forward<Rest>(rest)...);
}

template<typename... Args>
[[noreturn]] void configError(const QString &format, Args &&...args)
{
    QString message = format;
    configErrorArgs(message, std::forward<Args>(args)...);
    configErrorImpl(message);
}

} // namespace

Config::Config()
{
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("h"), QStringLiteral("help")}));
    parser.addVersionOption();

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("l"), QStringLiteral("host")}, {}, QStringLiteral("host url")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("n"), QStringLiteral("name")}, {}, QStringLiteral("Screen Name")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("s"), QStringLiteral("playing-style")}, {}, QStringLiteral("playing style")));

    parser.process(*qApp);

    if (!parser.positionalArguments().isEmpty())
        configError(QStringLiteral("Unknown argument: %1"), parser.positionalArguments().join(QStringLiteral(", ")));

    if (parser.isSet(QStringLiteral("h"))) {
        std::cout << qPrintable(helpText) << std::flush;
        std::exit(0);
    }

    read_(&parser);
}

void Config::read_(QCommandLineParser *parser)
{
    if (!parser->isSet(QStringLiteral("host")))
        configError(QStringLiteral("Host is required."));
    host_ = parser->value(QStringLiteral("host"));

    if (parser->isSet(QStringLiteral("name")))
        name_ = parser->value(QStringLiteral("name"));

    playingStyle_ = QStringLiteral("knifePreferred");
    if (parser->isSet(QStringLiteral("playing-style")))
        playingStyle_ = parser->value(QStringLiteral("playing-style"));

    // Validate the resolved style uniformly (default included), so the
    // whitelist in Bot::styleExist is the single source of truth.
    if (!Bot::styleExist(playingStyle_))
        configError(QStringLiteral("Specified playing style does not exist."));
}

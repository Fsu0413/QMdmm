// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"

#include <QMdmmSettings>

#include <QCommandLineParser>

#include <iostream>

static const QString helpText = QStringLiteral(R"help(

-h --help
-v --version

Connection options:
-l --host=<host url / connection string>
-n --name=<screen name>

Bot options:
-s --playing-style=<styles>

)help");

namespace {

[[noreturn]] void configErrorImpl(const QString &message)
{
    std::cerr << qPrintable(message) << '\n' << std::flush;
    qWarning().noquote() << message;

    std::exit(3);
}

template<typename... Args>
[[noreturn]] void configError(const QString &format, Args &&...args)
{
    QString message = format;
    (void)std::initializer_list<int> {(message = message.arg(args), 0)...};
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

    if (!parser.unknownOptionNames().isEmpty())
        configError(QStringLiteral("Unknown option: %1"), parser.unknownOptionNames().join(QStringLiteral(", ")));

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
        configError(QStringLiteral("..."));
    host_ = parser->value(QStringLiteral("host"));

    if (parser->isSet(QStringLiteral("name")))
        name_ = parser->value(QStringLiteral("name"));

    if (parser->isSet(QStringLiteral("playing-style")))
        playingStyle_ = parser->value(QStringLiteral("playing-style"));
}

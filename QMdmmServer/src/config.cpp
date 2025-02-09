// SPDX-License-Identifier: AGPL-3.0-or-later

#include "config.h"

#include <QMdmmSettings>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>

static const QString helpText = QStringLiteral(R"help(

-h --help
-v --version

TCP options:
-t --tcp=<on/off> Enable TCP
-p --tcp-port=<port> TCP listen port

LocalSocket options:
-l --local=<on/off> Enable local socket
-L --local-name=<name> local socket name

WebSocket options:
-w --websocket=<on/off> Enable WebSocket
-W --websocket-name=<name> WebSocket name
-P --websocket-port=<port> WebSocket listen port

LogicRunner configurations:
-n --players=<2~> player number per Room
-o --timeout=<0,15~> operation timeout

Logic configurations:
-s --slash=, --knife=<1~> initial knife (slash) damage
-S --maximum-slash=, --maximum-knife=<5~> maximum knife (slash) damage
-k --kick=, --horse=<2~> initial horse (kick) damage
-K --maximum-kick=, --maximum-horse=<5~> maximum horse (kick) damage
-m --maxhp=<7~> initial max hp
-M --maximum-maxhp=<10~> maximum max hp
-r --punish-hp-modifier=<0,2~> punish hp modifier
-R --punish-hp-round-strategy=<strategy> punish hp round strategy (see below)
-z --zero-hp-as-dead=<true/false> treat zero hp as dead
-f --enable-let-move=<true/false> enable "let move"
-i --can-buy-only-in-initial-city=<true/false> can buy only in initial city

Configuration saves / examining (Only one of following can be specified):
-c --save-configuration Save configuration specified in command line and exits
-C --save-global-configuration Save configuration specified in command line to system global and exits
-d --show-current-configuration Show current configuration as JSON

)help");

// NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if)
#if 0
ab  e g  j    q  u  xy
AB DEFGHIJ NO Q TUV XYZ
01
#endif

Config::Config()
{
    QMdmmSettings setting;

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);

    // parser_.addHelpOption(); // do not fit my need
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("h"), QStringLiteral("help")}));
    parser.addVersionOption();

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("t"), QStringLiteral("tcp")}, {}, QStringLiteral("on/off")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("p"), QStringLiteral("tcp-port")}, {}, QStringLiteral("port")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("l"), QStringLiteral("local")}, {}, QStringLiteral("on/off")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("L"), QStringLiteral("local-name")}, {}, QStringLiteral("name")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("w"), QStringLiteral("websocket")}, {}, QStringLiteral("on/off")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("W"), QStringLiteral("websocket-name")}, {}, QStringLiteral("name")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("P"), QStringLiteral("websocket-port")}, {}, QStringLiteral("port")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("n"), QStringLiteral("players")}, {}, QStringLiteral("2~")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("2")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("3")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("4")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("5")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("6")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("7")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("8")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("9")}));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("o"), QStringLiteral("timeout")}, {}, QStringLiteral("0,15~")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("s"), QStringLiteral("slash"), QStringLiteral("knife")}, {}, QStringLiteral("1~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("S"), QStringLiteral("maximum-slash"), QStringLiteral("maximum-knife")}, {}, QStringLiteral("5~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("k"), QStringLiteral("kick"), QStringLiteral("horse")}, {}, QStringLiteral("2~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("K"), QStringLiteral("maximum-kick"), QStringLiteral("maximum-horse")}, {}, QStringLiteral("5~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("m"), QStringLiteral("maxhp")}, {}, QStringLiteral("7~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("M"), QStringLiteral("maximum-maxhp")}, {}, QStringLiteral("10~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("r"), QStringLiteral("punish-hp-modifier")}, {}, QStringLiteral("0,2~")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("R"), QStringLiteral("punish-hp-round-strategy")}, {}, QStringLiteral("strategy")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("z"), QStringLiteral("zero-hp-as-dead")}, {}, QStringLiteral("true/false")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("f"), QStringLiteral("enable-let-move")}, {}, QStringLiteral("true/false")));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("i"), QStringLiteral("can-buy-only-in-initial-city")}, {}, QStringLiteral("true/false")));

    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("c"), QStringLiteral("save-configuration")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("C"), QStringLiteral("save-global-configuration")}));
    parser.addOption(QCommandLineOption(QStringList {QStringLiteral("d"), QStringLiteral("show-current-configuration")}));

    parser.process(*qApp);

    if (!parser.unknownOptionNames().isEmpty())
        qFatal("Unknown option: %s", qPrintable(parser.unknownOptionNames().join(QStringLiteral(", "))));

    if (!parser.positionalArguments().isEmpty())
        qFatal("Unknown argument: %s", qPrintable(parser.positionalArguments().join(QStringLiteral(", "))));

    if (parser.isSet(QStringLiteral("h"))) {
        std::cout << qPrintable(helpText) << std::flush;
        ::exit(0);
    }

    QMdmmSettings::Instance toSave = QMdmmSettings::Specified;

    if (parser.isSet(QStringLiteral("c"))) {
        if (parser.isSet(QStringLiteral("C")))
            qFatal("It is not supported to save both per-user configuration and global configuration at one time. Exiting.");
        toSave = QMdmmSettings::PerUser;
    } else if (parser.isSet(QStringLiteral("C"))) {
        toSave = QMdmmSettings::Global;
    }

    read_(&setting, &parser);
    if (toSave != QMdmmSettings::Specified)
        save_(&setting, toSave);
    if (parser.isSet(QStringLiteral("d")))
        show_();
}

namespace {

inline std::optional<bool> stringToBool(const QString &value)
{
    // clang-format off
    static const QStringList falseValues {
        QStringLiteral("0"),
        QStringLiteral("false"),
        QStringLiteral("no"),
        QStringLiteral("n"),
        QStringLiteral("off"),
        QStringLiteral("disable"),
        QStringLiteral("disabled"),
    };
    static const QStringList trueValues {
        QStringLiteral("1"),
        QStringLiteral("true"),
        QStringLiteral("yes"),
        QStringLiteral("y"),
        QStringLiteral("on"),
        QStringLiteral("enable"),
        QStringLiteral("enabled"),
    };
    // clang-format on

    foreach (const QString &falseValue, falseValues) {
        if (value.compare(falseValue, Qt::CaseInsensitive) == 0)
            return false;
    }
    foreach (const QString &trueValue, trueValues) {
        if (value.compare(trueValue, Qt::CaseInsensitive) == 0)
            return true;
    }
    return std::nullopt;
}

inline std::optional<int> stringToInt(const QString &value)
{
    bool ok = false;
    int result = value.toInt(&ok);
    if (!ok)
        return std::nullopt;

    return result;
}

inline std::optional<uint16_t> stringToUint16(const QString &value)
{
    bool ok = false;
    unsigned int result = value.toUInt(&ok);
    if (!ok)
        return std::nullopt;
    if (result > UINT16_MAX)
        return std::nullopt;

    return result;
}

inline std::optional<QMdmmLogicConfiguration::PunishHpRoundStrategy> stringToPunishHpRoundStrategy(const QString &value)
{
    static const QHash<QString, QMdmmLogicConfiguration::PunishHpRoundStrategy> strategyHash {
        std::make_pair(QStringLiteral("RoundDown"), QMdmmLogicConfiguration::RoundDown),
        std::make_pair(QStringLiteral("RoundToNearest45"), QMdmmLogicConfiguration::RoundToNearest45),
        std::make_pair(QStringLiteral("RoundUp"), QMdmmLogicConfiguration::RoundUp),
        std::make_pair(QStringLiteral("PlusOne"), QMdmmLogicConfiguration::PlusOne),
    };

    for (QHash<QString, QMdmmLogicConfiguration::PunishHpRoundStrategy>::const_iterator it = strategyHash.constBegin(); it != strategyHash.constEnd(); ++it) {
        if (it.key().compare(value, Qt::CaseInsensitive) == 0)
            return it.value();
    }

    return std::nullopt;
}

inline QString boolToString(bool value)
{
    return value ? QStringLiteral("on") : QStringLiteral("off");
}

inline QString intToString(int value)
{
    return QString::number(value);
}

inline QString uint16ToString(uint16_t value)
{
    return QString::number(static_cast<unsigned int>(value));
}

inline QString punishHpRoundStrategyToString(QMdmmLogicConfiguration::PunishHpRoundStrategy value)
{
    static const QHash<QMdmmLogicConfiguration::PunishHpRoundStrategy, QString> strategyHash {
        std::make_pair(QMdmmLogicConfiguration::RoundDown, QStringLiteral("RoundDown")),
        std::make_pair(QMdmmLogicConfiguration::RoundToNearest45, QStringLiteral("RoundToNearest45")),
        std::make_pair(QMdmmLogicConfiguration::RoundUp, QStringLiteral("RoundUp")),
        std::make_pair(QMdmmLogicConfiguration::PlusOne, QStringLiteral("PlusOne")),
    };

    return strategyHash.value(value, QString());
}

} // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity,readability-function-size)
void Config::read_(QMdmmSettings *setting, QCommandLineParser *parser)
{
#define CONFIG_ITEM(type, conf, settingName, parserConvert, ValueName)                  \
    do {                                                                                \
        QString s;                                                                      \
        int f = 0;                                                                      \
        if (parser->isSet(QStringLiteral(settingName))) {                               \
            f = 1;                                                                      \
            s = parser->value(QStringLiteral(settingName));                             \
        } else if (setting->contains(QStringLiteral(settingName))) {                    \
            f = 2;                                                                      \
            s = setting->value(QStringLiteral(settingName)).toString();                 \
        }                                                                               \
        if (f != 0) {                                                                   \
            std::optional<type> v = parserConvert(s);                                   \
            if (v.has_value()) {                                                        \
                (conf).set##ValueName(v.value());                                       \
            } else {                                                                    \
                const char *from = nullptr;                                             \
                if (f == 1)                                                             \
                    from = "command line";                                              \
                else if (f == 2)                                                        \
                    from = "config file";                                               \
                else                                                                    \
                    from = "unknown config";                                            \
                qFatal("Config item %s (from %s) can't be parsed.", settingName, from); \
            }                                                                           \
        }                                                                               \
    } while (false)

    setting->beginGroup(QStringLiteral("server"));

    CONFIG_ITEM(bool, serverConfiguration_, "tcp", stringToBool, TcpEnabled);
    CONFIG_ITEM(uint16_t, serverConfiguration_, "tcp-port", stringToUint16, TcpPort);
    CONFIG_ITEM(bool, serverConfiguration_, "local", stringToBool, LocalEnabled);
    CONFIG_ITEM(QString, serverConfiguration_, "local-name", , LocalSocketName);
    CONFIG_ITEM(bool, serverConfiguration_, "websocket", stringToBool, WebsocketEnabled);
    CONFIG_ITEM(QString, serverConfiguration_, "websocket-name", , WebsocketName);
    CONFIG_ITEM(uint16_t, serverConfiguration_, "websocket-port", stringToUint16, WebsocketPort);

    setting->endGroup();

    setting->beginGroup(QStringLiteral("logic"));

    {
        int players = 0;

        // clang-format off
        static const std::initializer_list<QString> shortForms {
            QStringLiteral("2"),
            QStringLiteral("3"),
            QStringLiteral("4"),
            QStringLiteral("5"),
            QStringLiteral("6"),
            QStringLiteral("7"),
            QStringLiteral("8"),
            QStringLiteral("9"),
        };
        // clang-format on

        for (size_t i = 0; i < shortForms.size(); ++i) {
            if (parser->isSet(std::data(shortForms)[i])) {
                if (players == 0)
                    players = (int)(i + 2);
                else
                    qFatal("-%d can't be specified alongwith -%d", (int)(i + 2), players);
            }
        }

        if (players != 0) {
            if (parser->isSet(QStringLiteral("players")))
                qFatal("-%d can't be specified alongwith -n / --players", players);

            logicConfiguration_.setPlayerNumPerRoom(players);
        } else {
            CONFIG_ITEM(int, logicConfiguration_, "players", stringToInt, PlayerNumPerRoom);
        }
    }

    CONFIG_ITEM(int, logicConfiguration_, "timeout", stringToInt, RequestTimeout);
    CONFIG_ITEM(int, logicConfiguration_, "slash", stringToInt, InitialKnifeDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-slash", stringToInt, MaximumKnifeDamage);
    CONFIG_ITEM(int, logicConfiguration_, "kick", stringToInt, InitialHorseDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-kick", stringToInt, MaximumHorseDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maxhp", stringToInt, InitialMaxHp);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-maxhp", stringToInt, MaximumMaxHp);
    CONFIG_ITEM(int, logicConfiguration_, "punish-hp-modifier", stringToInt, PunishHpModifier);
    CONFIG_ITEM(QMdmmLogicConfiguration::PunishHpRoundStrategy, logicConfiguration_, "punish-hp-round-strategy", stringToPunishHpRoundStrategy, PunishHpRoundStrategy);
    CONFIG_ITEM(bool, logicConfiguration_, "zero-hp-as-dead", stringToBool, ZeroHpAsDead);
    CONFIG_ITEM(bool, logicConfiguration_, "enable-let-move", stringToBool, EnableLetMove);
    CONFIG_ITEM(bool, logicConfiguration_, "can-buy-only-in-initial-city", stringToBool, CanBuyOnlyInInitialCity);

    setting->endGroup();

#undef CONFIG_ITEM
}

void Config::save_(QMdmmSettings *setting, QMdmmSettings::Instance toSave)
{
    // NOLINTBEGIN(bugprone-macro-parentheses)

#define CONFIG_ITEM(type, conf, settingName, settingConvert, valueName) \
    do {                                                                \
        type v = conf.valueName();                                      \
        QString s = settingConvert(v);                                  \
        setting->setValue(QStringLiteral(settingName), s);              \
    } while (false)

    // NOLINTEND(bugprone-macro-parentheses)

    setting->beginGroup(QStringLiteral("server"));

    CONFIG_ITEM(bool, serverConfiguration_, "tcp", boolToString, tcpEnabled);
    CONFIG_ITEM(uint16_t, serverConfiguration_, "tcp-port", uint16ToString, tcpPort);
    CONFIG_ITEM(bool, serverConfiguration_, "local", boolToString, localEnabled);
    CONFIG_ITEM(QString, serverConfiguration_, "local-name", , localSocketName);
    CONFIG_ITEM(bool, serverConfiguration_, "websocket", boolToString, websocketEnabled);
    CONFIG_ITEM(QString, serverConfiguration_, "websocket-name", , websocketName);
    CONFIG_ITEM(uint16_t, serverConfiguration_, "websocket-port", uint16ToString, websocketPort);

    setting->endGroup();

    setting->beginGroup(QStringLiteral("logic"));

    CONFIG_ITEM(int, logicConfiguration_, "players", intToString, playerNumPerRoom);
    CONFIG_ITEM(int, logicConfiguration_, "timeout", intToString, requestTimeout);
    CONFIG_ITEM(int, logicConfiguration_, "slash", intToString, initialKnifeDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-slash", intToString, maximumKnifeDamage);
    CONFIG_ITEM(int, logicConfiguration_, "kick", intToString, initialHorseDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-kick", intToString, maximumHorseDamage);
    CONFIG_ITEM(int, logicConfiguration_, "maxhp", intToString, initialMaxHp);
    CONFIG_ITEM(int, logicConfiguration_, "maximum-maxhp", intToString, maximumMaxHp);
    CONFIG_ITEM(int, logicConfiguration_, "punish-hp-modifier", intToString, punishHpModifier);
    CONFIG_ITEM(QMdmmLogicConfiguration::PunishHpRoundStrategy, logicConfiguration_, "punish-hp-round-strategy", punishHpRoundStrategyToString, punishHpRoundStrategy);
    CONFIG_ITEM(bool, logicConfiguration_, "zero-hp-as-dead", boolToString, zeroHpAsDead);
    CONFIG_ITEM(bool, logicConfiguration_, "enable-let-move", boolToString, enableLetMove);
    CONFIG_ITEM(bool, logicConfiguration_, "can-buy-only-in-initial-city", boolToString, canBuyOnlyInInitialCity);

    setting->endGroup();

#undef CONFIG_ITEM

    ::exit(static_cast<int>(setting->saveConfig(toSave)));
}

void Config::show_()
{
    QJsonObject ob;
    ob.insert(QStringLiteral("ServerConfiguration"), serverConfiguration_);
    ob.insert(QStringLiteral("LogicConfiguration"), logicConfiguration_);

    QByteArray arr = QJsonDocument(ob).toJson(QJsonDocument::Indented);
    std::cout << arr.constData() << '\n' << std::flush;

    ::exit(0);
}

const QMdmmServerConfiguration &Config::serverConfiguration() const
{
    return serverConfiguration_;
}

const QMdmmLogicConfiguration &Config::logicConfiguration() const
{
    return logicConfiguration_;
}

#include "qmdmmlogic.h"

#include "qmdmmagent.h"
#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QJsonObject>
#include <QJsonValue>

QJsonValue QMdmmLogicConfiguration::serialize() const
{
    QJsonObject ob;

#define CONF(member, type) ob.insert(QStringLiteral(#member), static_cast<type>(this->member))

    CONF(playerNumPerRoom, int);
    CONF(initialKnifeDamage, int);
    CONF(maximumKnifeDamage, int);
    CONF(initialHorseDamage, int);
    CONF(maximumHorseDamage, int);
    CONF(initialMaxHp, int);
    CONF(maximumMaxHp, int);
    CONF(punishHpModifier, int);
    CONF(punishHpRoundStrategy, int);
    CONF(zeroHpAsDead, bool);

#undef CONF

    return ob;
}

bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    static const QMdmmLogicConfiguration defaults;

#define CONVERTPUNISHHPROUNDSTRATEGY(defaultValue) static_cast<PunishHpRoundStrategy>(v.toInt(defaultValue))
#define CONF(member, check, convert)                      \
    do {                                                  \
        QJsonValue v = ob.value(QStringLiteral(#member)); \
        if (!v.check())                                   \
            return false;                                 \
        member = convert(defaults.member);                \
    } while (false)

    CONF(playerNumPerRoom, isDouble, v.toInt);
    CONF(initialKnifeDamage, isDouble, v.toInt);
    CONF(maximumKnifeDamage, isDouble, v.toInt);
    CONF(initialHorseDamage, isDouble, v.toInt);
    CONF(maximumHorseDamage, isDouble, v.toInt);
    CONF(initialMaxHp, isDouble, v.toInt);
    CONF(maximumMaxHp, isDouble, v.toInt);
    CONF(punishHpModifier, isDouble, v.toInt);
    CONF(punishHpRoundStrategy, isDouble, CONVERTPUNISHHPROUNDSTRATEGY);
    CONF(zeroHpAsDead, isBool, v.toBool);

#undef CONF
#undef CONVERTPUNISHHPROUNDSTRATEGY

    return true;
}

void QMdmmLogic::run()
{
    QMdmmRoom *room = new QMdmmRoom(this);

    room->addPlayer(QStringLiteral("player1"));
    room->addPlayer(QStringLiteral("player2"));
    room->addPlayer(QStringLiteral("player3"));

    // logic main loop

    delete room;
}

QMdmmLogicRunner::QMdmmLogicRunner(QObject *parent)
    : QThread(parent)
{
}

QMdmmLogicRunner::~QMdmmLogicRunner() = default;

bool QMdmmLogicRunner::registerAgent(QMdmmAgent *agent)
{
    return false;
}

bool QMdmmLogicRunner::deregisterAgent(QMdmmAgent *agent)
{
    return false;
}

// bool QMdmmLogicRunner::incomingMessage(int todo)
// {
// }

// bool QMdmmLogicRunner::outgoingMessage(int todo)
// {
// }

// bool QMdmmLogicRunner::outgoingMessageFromLogic(int todo)
// {
// }

void QMdmmLogicRunner::run()
{
    QMdmmLogic logic;
    logic.run();
}

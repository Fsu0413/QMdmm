#include "qmdmmlogic.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QPointer>

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

struct QMdmmLogicPrivate
{
    QMdmmLogicConfiguration conf;
};

QMdmmLogic::QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmLogicPrivate {logicConfiguration})
{
}

QMdmmLogic::~QMdmmLogic()
{
    delete d;
}

const QMdmmLogicConfiguration &QMdmmLogic::configuration() const
{
    return d->conf;
}

void QMdmmLogic::run()
{

}

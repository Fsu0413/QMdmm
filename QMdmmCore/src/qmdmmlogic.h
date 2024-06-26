// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"
#include <QObject>

struct QMDMMCORE_EXPORT QMdmmLogicConfiguration
{
    int playerNumPerRoom = 3;

    int initialKnifeDamage = 1;
    int maximumKnifeDamage = 5;

    int initialHorseDamage = 2;
    int maximumHorseDamage = 10;

    int initialMaxHp = 10;
    int maximumMaxHp = 20;

    int punishHpModifier = 2; // 1 / punishHpModifier for punished HP when slashed in city, 0 for disable.
    enum PunishHpRoundStrategy
    {
        RoundDown, // 1.1 -> 1 1.4 -> 1, 1.5 -> 1, 1.9 -> 1, 2.0 -> 2
        RoundToNearest, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        RoundUp, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        PlusOne, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 3
    } punishHpRoundStrategy
        = RoundDown;

    bool zeroHpAsDead = true; // Treat one with hp value 0 as dead. If false only one with minus hp value are treated as dead

    [[nodiscard]] QJsonValue serialize() const;
    bool deserialize(const QJsonValue &value);
};

struct QMdmmLogicPrivate;

class QMDMMCORE_EXPORT QMdmmLogic : public QObject
{
    Q_OBJECT

public:
    QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogic() override;

    [[nodiscard]] const QMdmmLogicConfiguration &configuration() const;

    void run();

private:
    QMdmmLogicPrivate *const d;
};

#endif

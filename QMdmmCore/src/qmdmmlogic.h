// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"
#include <QThread>

class QMdmmAgent;

struct QMDMMCORE_EXPORT QMdmmLogicConfiguration
{
    int playerNumPerRoom = 1;

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

// This will be implemented using custom event queue.
// Qt signal-slot does not apply in this class.
class QMDMMCORE_EXPORT QMdmmLogic
{
public:
    void run();
};

class QMDMMCORE_EXPORT QMdmmLogicRunner final : public QThread
{
    Q_OBJECT

public:
    // Constuctor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    // pay attention
    QMdmmLogicRunner(QObject *parent = nullptr);
    ~QMdmmLogicRunner() override;

    // Functions to be called in Server thread
    bool registerAgent(QMdmmAgent *agent);
    bool deregisterAgent(QMdmmAgent *agent);

    // void start() (not overrided);

    void run() override;
};

#endif

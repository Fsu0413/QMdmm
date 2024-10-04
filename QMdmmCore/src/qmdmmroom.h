// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"

#include <QList>
#include <QObject>

class QMdmmPlayer;
class QMdmmLogic;

struct QMdmmRoomPrivate;

class QMDMMCORE_EXPORT QMdmmRoom final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmRoom(QMdmmLogic *logic);
    ~QMdmmRoom() override;

    [[nodiscard]] QMdmmLogic *logic();
    [[nodiscard]] const QMdmmLogic *logic() const;

    QMdmmPlayer *addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);
    [[nodiscard]] bool full() const;

    [[nodiscard]] QMdmmPlayer *player(const QString &playerName);
    [[nodiscard]] const QMdmmPlayer *player(const QString &playerName) const;

    [[nodiscard]] QList<QMdmmPlayer *> players();
    [[nodiscard]] QList<const QMdmmPlayer *> players() const;
    [[nodiscard]] QStringList playerNames() const;

    [[nodiscard]] QList<QMdmmPlayer *> alivePlayers();
    [[nodiscard]] QList<const QMdmmPlayer *> alivePlayers() const;
    [[nodiscard]] QStringList alivePlayerNames() const;
    [[nodiscard]] int alivePlayersCount() const;
    [[nodiscard]] bool isRoundOver() const;
    [[nodiscard]] bool isGameOver(QStringList *winnerPlayerNames = nullptr) const;

    void prepareForRoundStart();
    void resetUpgrades();

signals:
    void playerAdded(const QString &playerName, QPrivateSignal);
    void playerRemoved(const QString &playerName, QPrivateSignal);

private:
    QMdmmRoomPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmRoom)
};

#endif // QMDMMROOM_H

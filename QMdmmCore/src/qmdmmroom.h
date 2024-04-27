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
    explicit QMdmmRoom(QMdmmLogic *logic, QObject *parent = nullptr);
    ~QMdmmRoom() override;

    [[nodiscard]] QMdmmLogic *logic();
    [[nodiscard]] const QMdmmLogic *logic() const;

    bool addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);
    [[nodiscard]] bool full() const;

    [[nodiscard]] QMdmmPlayer *player(const QString &playerName) const;

    [[nodiscard]] QList<QMdmmPlayer *> players() const;
    [[nodiscard]] QStringList playerNames() const;

    [[nodiscard]] QList<QMdmmPlayer *> alivePlayers() const;
    [[nodiscard]] int alivePlayersCount() const;

private:
    QMdmmRoomPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmRoom)
};

#endif // QMDMMROOM_H

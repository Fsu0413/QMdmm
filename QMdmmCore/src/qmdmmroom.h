// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"

#include <QList>
#include <QObject>

struct QMdmmRoomPrivate;
class QMdmmPlayer;

class QMDMMCORE_EXPORT QMdmmRoom final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmRoom(QObject *parent = nullptr);
    ~QMdmmRoom() override;

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
    Q_DISABLE_COPY(QMdmmRoom)
};

#endif // QMDMMROOM_H

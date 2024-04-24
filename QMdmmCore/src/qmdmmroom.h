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
    bool full() const;

    QMdmmPlayer *player(const QString &playerName) const;

    QList<QMdmmPlayer *> players() const;
    QStringList playerNames() const;

    QList<QMdmmPlayer *> alivePlayers() const;
    int alivePlayersCount() const;

private:
    QMdmmRoomPrivate *const d;
    Q_DISABLE_COPY(QMdmmRoom)
};

#endif // QMDMMROOM_H

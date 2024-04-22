// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"
#include <string>
#include <vector>

struct QMdmmRoomPrivate;
class QMdmmPlayer;

class QMDMMCORE_EXPORT QMdmmRoom
{
public:
    QMdmmRoom();
    virtual ~QMdmmRoom();

    std::string addPlayer(QMdmmPlayer *player, const std::string &userName = std::string());
    bool full() const;

    bool removePlayer(QMdmmPlayer *player);
    bool removePlayer(const std::string &playerName);

    QMdmmPlayer *player(const std::string &playerName) const;

    std::vector<QMdmmPlayer *> players() const;
    std::vector<std::string> playerNames() const;

    std::vector<QMdmmPlayer *> alivePlayers() const;
    int alivePlayersCount() const;

private:
    QMdmmRoomPrivate *const d;
    QMDMM_DISABLE_COPY(QMdmmRoom)
};

#endif // QMDMMROOM_H

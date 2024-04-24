// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"
#include <string>
#include <vector>

struct QMdmmRoomPrivate;
class QMdmmPlayer;

class QMDMMCORE_EXPORT QMdmmRoom final
{
public:
    QMdmmRoom();
    virtual ~QMdmmRoom();

    bool addPlayer(const std::string &playerName);
    bool removePlayer(const std::string &playerName);
    bool full() const;

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

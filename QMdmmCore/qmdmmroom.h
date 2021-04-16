// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"
#include <string>
#include <vector>

struct QMdmmRoomPrivate;
class QMdmmPlayer;

using std::string;
using std::vector;

class QMDMMCORE_EXPORT QMdmmRoom
{
public:
    QMdmmRoom();
    virtual ~QMdmmRoom();

    string addPlayer(QMdmmPlayer *player, const string &userName = string());
    bool full() const;

    bool removePlayer(QMdmmPlayer *player);
    bool removePlayer(const string &playerName);

    QMdmmPlayer *player(const string &playerName) const;

    vector<QMdmmPlayer *> players() const;
    vector<string> playerNames() const;

    vector<QMdmmPlayer *> alivePlayers() const;
    int alivePlayersCount() const;

private:
    QMDMM_D(QMdmmRoom)
    QMDMM_DISABLE_COPY(QMdmmRoom)
};

#endif // QMDMMROOM_H

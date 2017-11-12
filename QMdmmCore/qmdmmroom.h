#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"
#include <string>
#include <vector>

struct QMdmRoomPrivate;
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
    string playerName(QMdmmPlayer *player) const;

    vector<QMdmmPlayer *> players() const;
    vector<string> playerNames() const;

private:
    QMDMM_D(QMdmmRoom)
    QMDMM_DISABLE_COPY(QMdmmRoom)
};

#endif // QMDMMROOM_H

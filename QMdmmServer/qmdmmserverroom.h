#ifndef QMDMMSERVERROOM_H
#define QMDMMSERVERROOM_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmProtocol>
#include <QMdmmCore/QMdmmRoom>

struct QMdmmServerRoomPrivate;
class QMdmmServerPlayer;

using std::string;

class QMDMMSERVER_EXPORT QMdmmServerRoom : public QMdmmRoom
{
public:
    QMdmmServerRoom();
    ~QMdmmServerRoom();

    struct QMDMMSERVER_EXPORT GameOverType
    {
        enum Type
        {
            RoundOver,
            GameOver,
            ErrorOver,

            NotOver
        };

        Type t;
        GameOverType() = delete;
        constexpr GameOverType(const GameOverType &other);
        constexpr GameOverType(const GameOverType::Type type);

        GameOverType &operator=(const GameOverType &other);

        bool operator==(const GameOverType &other) const;
    };

    static GameOverType RoundOver;
    static GameOverType GameOver;
    static GameOverType ErrorOver;

    void run();
    void notified(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);
    void replyed(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &notifyData);

private:
    QMDMM_D(QMdmmServerRoom)
};

#endif // QMDMMSERVERROOM_H

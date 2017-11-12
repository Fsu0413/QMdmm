#ifndef QMDMMSERVERROOM_H
#define QMDMMSERVERROOM_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmRoom>

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
};

#endif // QMDMMSERVERROOM_H

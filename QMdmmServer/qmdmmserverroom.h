#ifndef QMDMMSERVERROOM_H
#define QMDMMSERVERROOM_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmRoom>

class QMDMMSERVER_EXPORT QMdmmServerRoom : public QMdmmRoom
{
public:
    QMdmmServerRoom();
    ~QMdmmServerRoom();

    struct GameOver
    {
        char dummy;
    };
    struct RoundOver
    {
        char dummy;
    };

    void run();
};

#endif // QMDMMSERVERROOM_H

#include "qmdmmserverroom.h"

QMdmmServerRoom::GameOverType QMdmmServerRoom::RoundOver(QMdmmServerRoom::GameOverType::RoundOver);
QMdmmServerRoom::GameOverType QMdmmServerRoom::GameOver(QMdmmServerRoom::GameOverType::GameOver);
QMdmmServerRoom::GameOverType QMdmmServerRoom::ErrorOver(QMdmmServerRoom::GameOverType::ErrorOver);

struct QMdmmServerRoomPrivate
{
};

QMdmmServerRoom::QMdmmServerRoom()
    : d_ptr(new QMdmmServerRoomPrivate)
{
}

QMdmmServerRoom::~QMdmmServerRoom()
{
    QMDMMD(QMdmmServerRoom);
    delete d;
}

void QMdmmServerRoom::run()
{
    forever {
        try {
            // main run
            // 1. StoneScissorsCloth, continue if tie
            // 2. If multiple players won, deside the order to operate
            // 3. Operate
        } catch (const GameOverType &type) {
            if (type == RoundOver) {
                // update
            } else {
                // Prepare to exit thread
                break;
            }
        }
    }
}

constexpr QMdmmServerRoom::GameOverType::GameOverType(const QMdmmServerRoom::GameOverType &other)
    : t(other.t)
{
}

constexpr QMdmmServerRoom::GameOverType::GameOverType(const QMdmmServerRoom::GameOverType::Type type)
    : t(type)
{
}

QMdmmServerRoom::GameOverType &QMdmmServerRoom::GameOverType::operator=(const QMdmmServerRoom::GameOverType &other)
{
    t = other.t;
    return *this;
}

bool QMdmmServerRoom::GameOverType::operator==(const QMdmmServerRoom::GameOverType &other) const
{
    return t == other.t;
}

#include "qmdmmserverroom.h"

QMdmmServerRoom::QMdmmServerRoom()
{
}

QMdmmServerRoom::~QMdmmServerRoom()
{
}

void QMdmmServerRoom::run()
{
    try {
        forever {
            try {
                forever {
                    // round
                }
            } catch (QMdmmServerRoom::RoundOver) {
                // update
            }
        }
    } catch (QMdmmServerRoom::GameOver) {
        // gameover, exit thread
    }
}

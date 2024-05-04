#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"

#include <QObject>

#include <cstdint>

struct QMdmmServerConfiguration
{
    uint16_t port = 6366U;
};

class QMdmmServerPrivate;

class QMDMMSERVER_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent = nullptr);
    ~QMdmmServer() override;

private:
    QMdmmServerPrivate *const d;
};

#endif // QMDMMSERVER_H

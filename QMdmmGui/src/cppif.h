// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMGUI_CPPIF_H_
#define QMDMMGUI_CPPIF_H_

#include <QDebug>
#include <QObject>

class QMdmmCppIf : public QObject
{
    Q_OBJECT

public slots:
    void testSlot()
    {
        qDebug() << "testSlot is called";
    }
};

#endif
